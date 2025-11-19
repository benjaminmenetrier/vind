/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/LinearModel/LinearModelPyTorchDDL95.h"

#include <filesystem>

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"
#include "vind/Increment.h"

namespace vind {

// -----------------------------------------------------------------------------

static LinearModelMaker<LinearModelTorchPyDDL95> makerPyDDL95_("PyTorchDDL95");

// -----------------------------------------------------------------------------

LinearModelTorchPyDDL95::LinearModelTorchPyDDL95(const Geometry & geom,
                                                 const eckit::Configuration & config)
  : timeResolution_(config.getString("time step")),
  comm_(geom.getComm()) {
  oops::Log::trace() << classname() << "::LinearModelTorchPyDDL95 starting" << std::endl;

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

  // Define tensor options
  opts_ = torch::TensorOptions()
    .dtype(torch::kFloat64)
    .layout(torch::kStrided)
    .device(torch::kCPU)
    .requires_grad(false);

  // Define coordinates
  lonTTensor_ = torch::empty({ny, nx}, opts_);
  latTTensor_ = torch::empty({ny, nx}, opts_);
  auto lonTView = lonTTensor_.accessor<double, 2>();
  auto latTView = latTTensor_.accessor<double, 2>();
  const double deg2rad = M_PI/180.0;
  for (int jx = 0; jx < nx; ++jx) {
    for (int jy = 0; jy < ny; ++jy) {
      const atlas::PointLonLat pointLonLat = grid.lonlat(jx, jy);
      lonTView[jy][jx] = pointLonLat.lon()*deg2rad;
      latTView[jy][jx] = pointLonLat.lat()*deg2rad;
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
  cMaskTTensor_ = torch::zeros({ny, nx}, opts_);
  yMaskTTensor_ = torch::zeros({ny, nx}, opts_);
  auto cMaskTView = cMaskTTensor_.accessor<double, 2>();
  auto yMaskTView = yMaskTTensor_.accessor<double, 2>();
  for (int jx = 0; jx < nx; ++jx) {
    for (int jy = 0; jy < ny; ++jy) {
      if ((jx >= ixMin) && (jx <= ixMax) && (jy >= iyMin) & (jy <= iyMax)) {
        // Inside computation zone
        cMaskTView[jy][jx] = 1.0;

        if ((jy > iyMin) && (jy < iyMax)) {
          // Y-direction diffusion
          yMaskTView[jy][jx] = 1.0;
        }
      }
    }
  }

  oops::Log::trace() << classname() << "::LinearModelTorchPyDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

LinearModelTorchPyDDL95::~LinearModelTorchPyDDL95() {
  oops::Log::trace() << classname() << "::~LinearModelTorchPyDDL95 starting" << std::endl;

  // Destroy parameters
  params_.reset();

  // Finalize Python interpreter
  pythonInterpreter_.finalize();

  oops::Log::trace() << classname() << "::~LinearModelTorchPyDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelTorchPyDDL95::setTrajectory(const State & xx,
                                     State & xlr,
                                     const ModelAuxControl & xxAux) {
  oops::Log::trace() << classname() << "::setTrajectory starting" << std::endl;

  // Save trajectory
  traj_.insert({xlr.validTime(), xlr});

  oops::Log::trace() << classname() << "::setTrajectory done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelTorchPyDDL95::stepTL(Increment & dx,
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
    torch::Tensor trajTTensor = torch::empty({globalTraj.shape(1), ny, nx}, opts_);
    torch::Tensor incrTTensor = torch::empty({globalIncr.shape(1), ny, nx}, opts_);

    // Copy data to numpy array
    auto trajTView = trajTTensor.accessor<double, 3>();
    auto incrTView = incrTTensor.accessor<double, 3>();
    const auto trajView = atlas::array::make_view<double, 2>(globalTraj);
    auto incrView = atlas::array::make_view<double, 2>(globalIncr);
    int ix, iy;
    for (int jnode = 0; jnode < globalIncr.shape(0); ++jnode) {
      // Get X/Y indices
      grid.index2ij(jnode, ix, iy);
      for (int jlevel = 0; jlevel < globalIncr.shape(1); ++jlevel) {
        trajTView[jlevel][iy][ix] = trajView(jnode, jlevel);
        incrTView[jlevel][iy][ix] = incrView(jnode, jlevel);
      }
    }

    // Load python module
    const std::string moduleName = "PyTorchDDL95";
    pybind11::module_ exec = pybind11::module_::import(moduleName.c_str());

    // Execute time step
    pybind11::object result = exec.attr("stepTL")(*params_, lonTTensor_, latTTensor_, cMaskTTensor_,
      yMaskTTensor_, t, trajTTensor, incrTTensor);

    // Copy data from numpy array
    for (int jnode = 0; jnode < globalIncr.shape(0); ++jnode) {
      grid.index2ij(jnode, ix, iy);
      for (int jlevel = 0; jlevel < globalIncr.shape(1); ++jlevel) {
        incrView(jnode, jlevel) = incrTView[jlevel][iy][ix];
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

void LinearModelTorchPyDDL95::stepAD(Increment & dx,
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
    torch::Tensor trajTTensor = torch::empty({globalTraj.shape(1), ny, nx}, opts_);
    torch::Tensor incrTTensor = torch::empty({globalIncr.shape(1), ny, nx}, opts_);

    // Copy data to numpy array
    auto trajTView = trajTTensor.accessor<double, 3>();
    auto incrTView = incrTTensor.accessor<double, 3>();
    const auto trajView = atlas::array::make_view<double, 2>(globalTraj);
    auto incrView = atlas::array::make_view<double, 2>(globalIncr);
    int ix, iy;
    for (int jnode = 0; jnode < globalIncr.shape(0); ++jnode) {
      // Get X/Y indices
      grid.index2ij(jnode, ix, iy);
      for (int jlevel = 0; jlevel < globalIncr.shape(1); ++jlevel) {
        trajTView[jlevel][iy][ix] = trajView(jnode, jlevel);
        incrTView[jlevel][iy][ix] = incrView(jnode, jlevel);
      }
    }

    // Load python module
    const std::string moduleName = "PyTorchDDL95";
    pybind11::module_ exec = pybind11::module_::import(moduleName.c_str());

    // Execute time step
    pybind11::object result = exec.attr("stepAD")(*params_, lonTTensor_, latTTensor_, cMaskTTensor_,
      yMaskTTensor_, t, trajTTensor, incrTTensor);

    // Copy data from numpy array
    for (int jnode = 0; jnode < globalIncr.shape(0); ++jnode) {
      grid.index2ij(jnode, ix, iy);
      for (int jlevel = 0; jlevel < globalIncr.shape(1); ++jlevel) {
        incrView(jnode, jlevel) = incrTView[jlevel][iy][ix];
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

void LinearModelTorchPyDDL95::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "PyTorchDDL95 linear model:" << std::endl;
  os << "- dt: " << timeResolution_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
