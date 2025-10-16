/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/LinearModel/LinearModelNumPyDDL95.h"

#include <filesystem>

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"
#include "vind/Increment.h"

namespace vind {

// -----------------------------------------------------------------------------

static LinearModelMaker<LinearModelNumPyDDL95> makerPyDDL95_("NumPyDDL95");

// -----------------------------------------------------------------------------

LinearModelNumPyDDL95::LinearModelNumPyDDL95(const Geometry & geom,
                                             const eckit::Configuration & config)
  : timeResolution_(config.getString("time step")),
  comm_(geom.getComm()) {
  oops::Log::trace() << classname() << "::LinearModelNumPyDDL95 starting" << std::endl;

  // Initialize Python interpreter
  pythonInterpreter_.initialize();

  // Get executable directory
#if defined(_MSC_VER)
  wchar_t path[FILENAME_MAX] = {0};
  GetModuleFileNameW(nullptr, path, FILENAME_MAX);
  const std::string pythonDir = std::filesystem::path(path).parent_path().string();
#else
  char path[FILENAME_MAX];
  ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
  const std::string pythonDir = std::filesystem::path(std::string(path, (count > 0) ? count : 0))
    .parent_path().string();
#endif

  // Insert python module into PYTHONPATH
  pybind11::module_ sys = pybind11::module_::import("sys");
  sys.attr("path").attr("insert")(1, pythonDir.c_str());

  // Create data dictionary
  params_.reset(new pybind11::dict());

  // Model parameters
  (*params_)["dt"] = timeResolution_.toSeconds();
  (*params_)["dti"] = timeResolution_.toSeconds()/36000.0;
  (*params_)["F"] = 8.0;
  (*params_)["omega"] = 2.0*M_PI/(24.0*3600.0);
  (*params_)["nu"] = 1.0;

  // Get dimensions
  const atlas::RegularGrid & grid = geom.grid();
  const int nx = grid.nx();
  const int ny = grid.ny();
  (*params_)["nx"] = grid.nx();
  (*params_)["ny"] = grid.ny();

  // Get computation zone boundaries
  if (grid.periodic()) {
    (*params_)["ixMin"] = 0;
    (*params_)["ixMax"] = nx-1;
  } else {
    (*params_)["ixMin"] = 2;
    (*params_)["ixMax"] = nx-2;
  }
  if (geom.gridType() == "regional") {
    (*params_)["iyMin"] = 0;
    (*params_)["iyMax"] = ny-1;
  } else {
    (*params_)["iyMin"] = 1;
    (*params_)["iyMax"] = ny-2;
  }

  // Define coordinates
  lonNArray_.reset(new pybind11::array_t<double>({ny, nx}));
  latNArray_.reset(new pybind11::array_t<double>({ny, nx}));
  auto lonNView = lonNArray_->mutable_unchecked<2>();
  auto latNView = latNArray_->mutable_unchecked<2>();
  const double deg2rad = M_PI/180.0;
  for (int jx = 0; jx < nx; ++jx) {
    for (int jy = 0; jy < ny; ++jy) {
      const atlas::PointLonLat pointLonLat = grid.lonlat(jx, jy);
      lonNView(jy, jx) = pointLonLat.lon()*deg2rad;
      latNView(jy, jx) = pointLonLat.lat()*deg2rad;
    }
  }

  oops::Log::trace() << classname() << "::LinearModelNumPyDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

LinearModelNumPyDDL95::~LinearModelNumPyDDL95() {
  oops::Log::trace() << classname() << "::~LinearModelNumPyDDL95 starting" << std::endl;

  // Destroy parameters
  params_.reset();

  // Finalize Python interpreter
  pythonInterpreter_.finalize();

  oops::Log::trace() << classname() << "::~LinearModelNumPyDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelNumPyDDL95::setTrajectory(const State & xx,
                                     State & xlr,
                                     const ModelAuxControl & xxAux) {
  oops::Log::trace() << classname() << "::setTrajectory starting" << std::endl;

  // Save trajectory
  traj_.insert({xlr.validTime(), xlr});

  oops::Log::trace() << classname() << "::setTrajectory done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelNumPyDDL95::stepTL(Increment & dx,
                                   const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepTL starting" << std::endl;

  // Get geometry
  const Geometry & geom(dx.geometry());

  // Assert number of variables
  ASSERT(dx.variables().size() == 1);
  const oops::Variable var = dx.variables()[0];

  // Global trajectory
  auto globalTraj = geom.functionSpace().createField<double>(atlas::option::name(var.name())
    | atlas::option::levels(var.getLevels()) | atlas::option::global());

  // Global increment
  auto globalIncr = geom.functionSpace().createField<double>(atlas::option::name(var.name())
    | atlas::option::levels(var.getLevels()) | atlas::option::global());

  // Gather data on main processor
  geom.functionSpace().gather(traj_.at(dx.validTime()).fieldSet()[var.name()], globalTraj);
  geom.functionSpace().gather(dx.fieldSet()[var.name()], globalIncr);

  // Get valid time components
  int year, month, day, hour, minute, second;
  dx.validTime().toYYYYMMDDhhmmss(year, month, day, hour, minute, second);

  // Get number of seconds since 00:00:00
  const double t = static_cast<double>(hour*3600+minute*60+second);

  if (comm_.rank() == 0) {
    // Get grid
    const atlas::StructuredGrid grid(geom.grid());

    // Create numpy arrays
    const int nx = (*params_)["nx"].cast<size_t>();
    const int ny = (*params_)["ny"].cast<size_t>();
    pybind11::array_t<double> trajNArray({globalTraj.shape(1), ny, nx});
    pybind11::array_t<double> incrNArray({globalIncr.shape(1), ny, nx});

    // Copy data to numpy array
    auto trajNView = trajNArray.mutable_unchecked<3>();
    auto incrNView = incrNArray.mutable_unchecked<3>();
    const auto trajView = atlas::array::make_view<double, 2>(globalTraj);
    auto incrView = atlas::array::make_view<double, 2>(globalIncr);
    int ix, iy;
    for (int jnode = 0; jnode < globalIncr.shape(0); ++jnode) {
      // Get X/Y indices
      grid.index2ij(jnode, ix, iy);
      for (int jlevel = 0; jlevel < globalIncr.shape(1); ++jlevel) {
        trajNView(jlevel, iy, ix) = trajView(jnode, jlevel);
        incrNView(jlevel, iy, ix) = incrView(jnode, jlevel);
      }
    }

    // Load python module
    const std::string moduleName = "NumPyDDL95";
    pybind11::module_ exec = pybind11::module_::import(moduleName.c_str());

    // Execute time step
    pybind11::object result = exec.attr("stepTL")(*params_, *lonNArray_, *latNArray_, t,
      trajNArray, incrNArray);

    // Copy data from numpy array
    for (int jnode = 0; jnode < globalIncr.shape(0); ++jnode) {
      grid.index2ij(jnode, ix, iy);
      for (int jlevel = 0; jlevel < globalIncr.shape(1); ++jlevel) {
        incrView(jnode, jlevel) = incrNView(jlevel, iy, ix);
      }
    }
  }

  // Scatter data from main processor
  geom.functionSpace().scatter(globalIncr, dx.fieldSet()[var.name()]);

  // Update valid time
  dx.updateTime(timeResolution_);

  oops::Log::trace() << classname() << "::stepTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelNumPyDDL95::stepAD(Increment & dx,
                                   const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepAD starting" << std::endl;

  // Get geometry
  const Geometry & geom(dx.geometry());

  // Assert number of variables
  ASSERT(dx.variables().size() == 1);
  const oops::Variable var = dx.variables()[0];

  // Global trajectory
  auto globalTraj = geom.functionSpace().createField<double>(atlas::option::name(var.name())
    | atlas::option::levels(var.getLevels()) | atlas::option::global());

  // Global increment
  auto globalIncr = geom.functionSpace().createField<double>(atlas::option::name(var.name())
    | atlas::option::levels(var.getLevels()) | atlas::option::global());

  // Gather data on main processor
  geom.functionSpace().gather(traj_.at(dx.validTime()-timeResolution_).fieldSet()[var.name()],
    globalTraj);
  geom.functionSpace().gather(dx.fieldSet()[var.name()], globalIncr);

  // Get valid time components
  int year, month, day, hour, minute, second;
  (dx.validTime()-timeResolution_).toYYYYMMDDhhmmss(year, month, day, hour, minute, second);

  // Get number of seconds since 00:00:00
  const double t = static_cast<double>(hour*3600+minute*60+second);

  if (comm_.rank() == 0) {
    // Get grid
    const atlas::StructuredGrid grid(geom.grid());

    // Create numpy arrays
    const int nx = (*params_)["nx"].cast<size_t>();
    const int ny = (*params_)["ny"].cast<size_t>();
    pybind11::array_t<double> trajNArray({globalTraj.shape(1), ny, nx});
    pybind11::array_t<double> incrNArray({globalIncr.shape(1), ny, nx});

    // Copy data to numpy array
    auto trajNView = trajNArray.mutable_unchecked<3>();
    auto incrNView = incrNArray.mutable_unchecked<3>();
    const auto trajView = atlas::array::make_view<double, 2>(globalTraj);
    auto incrView = atlas::array::make_view<double, 2>(globalIncr);
    int ix, iy;
    for (int jnode = 0; jnode < globalIncr.shape(0); ++jnode) {
      // Get X/Y indices
      grid.index2ij(jnode, ix, iy);
      for (int jlevel = 0; jlevel < globalIncr.shape(1); ++jlevel) {
        trajNView(jlevel, iy, ix) = trajView(jnode, jlevel);
        incrNView(jlevel, iy, ix) = incrView(jnode, jlevel);
      }
    }

    // Load python module
    const std::string moduleName = "NumPyDDL95";
    pybind11::module_ exec = pybind11::module_::import(moduleName.c_str());

    // Execute time step
    pybind11::object result = exec.attr("stepAD")(*params_, *lonNArray_, *latNArray_, t,
      trajNArray, incrNArray);

    // Copy data from numpy array
    for (int jnode = 0; jnode < globalIncr.shape(0); ++jnode) {
      grid.index2ij(jnode, ix, iy);
      for (int jlevel = 0; jlevel < globalIncr.shape(1); ++jlevel) {
        incrView(jnode, jlevel) = incrNView(jlevel, iy, ix);
      }
    }
  }

  // Scatter data from main processor
  geom.functionSpace().scatter(globalIncr, dx.fieldSet()[var.name()]);

  // Update valid time
  dx.updateTime(-timeResolution_);

  oops::Log::trace() << classname() << "::stepAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelNumPyDDL95::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "PyDDL95 linear model:" << std::endl;
  os << "- dt: " << timeResolution_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
