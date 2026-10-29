/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/LinearModel/LinearModelPyAtlasDDL95.h"

#include <filesystem>

#include "atlas/util/Constants.h"

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"
#include "vind/Increment.h"

namespace vind {

// -----------------------------------------------------------------------------

static LinearModelMaker<LinearModelPyAtlasDDL95> makerPyDDL95_("PyAtlasDDL95");

// -----------------------------------------------------------------------------

LinearModelPyAtlasDDL95::LinearModelPyAtlasDDL95(const Geometry & geom,
                                                 const eckit::Configuration & config)
  : timeResolution_(config.getString("time step")),
  comm_(geom.getComm()) {
  oops::Log::trace() << classname() << "::LinearModelPyAtlasDDL95 starting" << std::endl;

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

  // Define coordinates
  atlas::Field lonField("lon", atlas::array::make_datatype<double>(),
    atlas::array::make_shape(ny, nx));
  geom_.add(lonField);
  atlas::Field latField("lat", atlas::array::make_datatype<double>(),
    atlas::array::make_shape(ny, nx));
  geom_.add(latField);
  auto lonView = atlas::array::make_view<double, 2>(lonField);
  auto latView = atlas::array::make_view<double, 2>(latField);
  for (int jx = 0; jx < nx; ++jx) {
    for (int jy = 0; jy < ny; ++jy) {
      const atlas::PointLonLat pointLonLat = grid.lonlat(jx, jy);
      lonView(jy, jx) = pointLonLat.lon()*atlas::util::Constants::degreesToRadians();
      latView(jy, jx) = pointLonLat.lat()*atlas::util::Constants::degreesToRadians();
    }
  }

  // Compute masks
  int ixMin, ixMax, iyMin, iyMax;
  if (grid.periodic()) {
    ixMin = 0;
    ixMax = nx-1;
  } else {
    ixMin = 2;
    ixMax = nx-2;
  }
  if (geom.gridType() == "regional") {
    iyMin = 0;
    iyMax = ny-1;
  } else {
    iyMin = 1;
    iyMax = ny-2;
  }
  atlas::Field cMaskField("cMask", atlas::array::make_datatype<double>(),
    atlas::array::make_shape(ny, nx));
  geom_.add(cMaskField);
  atlas::Field yMaskField("yMask", atlas::array::make_datatype<double>(),
    atlas::array::make_shape(ny, nx));
  geom_.add(yMaskField);
  auto cMaskView = atlas::array::make_view<double, 2>(cMaskField);
  auto yMaskView = atlas::array::make_view<double, 2>(yMaskField);
  cMaskView.assign(0.0);
  yMaskView.assign(0.0);
  for (int jx = 0; jx < nx; ++jx) {
    for (int jy = 0; jy < ny; ++jy) {
      if ((jx >= ixMin) && (jx <= ixMax) && (jy >= iyMin) && (jy <= iyMax)) {
        // Inside computation zone
        cMaskView(jy, jx) = 1.0;

        if ((jy > iyMin) && (jy < iyMax)) {
          // Y-direction diffusion
          yMaskView(jy, jx) = 1.0;
        }
      }
    }
  }

  oops::Log::trace() << classname() << "::LinearModelPyAtlasDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

LinearModelPyAtlasDDL95::~LinearModelPyAtlasDDL95() {
  oops::Log::trace() << classname() << "::~LinearModelPyAtlasDDL95 starting" << std::endl;

  // Destroy parameters
  params_.reset();

  // Finalize Python interpreter
  pythonInterpreter_.finalize();

  oops::Log::trace() << classname() << "::~LinearModelPyAtlasDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPyAtlasDDL95::setTrajectory(const State & xx,
                                     State & xlr,
                                     const ModelAuxControl & xxAux) {
  oops::Log::trace() << classname() << "::setTrajectory starting" << std::endl;

  // Save trajectory
  traj_.insert({xlr.validTime(), xlr});

  oops::Log::trace() << classname() << "::setTrajectory done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPyAtlasDDL95::stepTL(Increment & dx,
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
    // Load python module
    const std::string moduleName = "PyAtlasDDL95";
    pybind11::module_ exec = pybind11::module_::import(moduleName.c_str());

    // Execute time step
    pybind11::object result = exec.attr("stepTL")(*params_, geom_, t, globalTraj, globalIncr);
  }

  // Scatter data from main processor
  geom.functionSpace().scatter(globalIncr, dx.fieldSet()[var.name()]);

  // Update valid time
  dx.updateTime(timeResolution_);

  oops::Log::trace() << classname() << "::stepTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPyAtlasDDL95::stepAD(Increment & dx,
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
    // Load python module
    const std::string moduleName = "PyAtlasDDL95";
    pybind11::module_ exec = pybind11::module_::import(moduleName.c_str());

    // Execute time step
    pybind11::object result = exec.attr("stepAD")(*params_, geom_, t, globalTraj, globalIncr);
  }

  // Scatter data from main processor
  geom.functionSpace().scatter(globalIncr, dx.fieldSet()[var.name()]);

  // Update valid time
  dx.updateTime(-timeResolution_);

  oops::Log::trace() << classname() << "::stepAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPyAtlasDDL95::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "PyDDL95 linear model:" << std::endl;
  os << "- dt: " << timeResolution_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
