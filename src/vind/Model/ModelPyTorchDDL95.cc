/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/Model/ModelPyTorchDDL95.h"

#include <filesystem>

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"
#include "vind/State.h"

namespace vind {

// -----------------------------------------------------------------------------

static ModelMaker<ModelPyTorchDDL95> makerPyDDL95_("PyTorchDDL95");

// -----------------------------------------------------------------------------

ModelPyTorchDDL95::ModelPyTorchDDL95(const Geometry & geom,
                                     const eckit::Configuration & config)
  : timeResolution_(config.getString("time step")),
  comm_(geom.getComm()) {
  oops::Log::trace() << classname() << "::ModelPyTorchDDL95 starting" << std::endl;

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
      if ((jx >= ixMin) && (jx <= ixMax) && (jy >= iyMin) && (jy <= iyMax)) {
        // Inside computation zone
        cMaskTView[jy][jx] = 1.0;

        if ((jy > iyMin) && (jy < iyMax)) {
          // Y-direction diffusion
          yMaskTView[jy][jx] = 1.0;
        }
      }
    }
  }

  oops::Log::trace() << classname() << "::ModelPyTorchDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

ModelPyTorchDDL95::~ModelPyTorchDDL95() {
  oops::Log::trace() << classname() << "::~ModelPyTorchDDL95 starting" << std::endl;

  // Destroy parameters
  params_.reset();

  // Finalize Python interpreter
  pythonInterpreter_.finalize();

  oops::Log::trace() << classname() << "::~ModelPyTorchDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelPyTorchDDL95::step(State & xx,
                             const ModelAuxControl & xxAux) const {
  oops::Log::trace() << classname() << "::step starting" << std::endl;

  // Get geometry
  const Geometry & geom(xx.geometry());

  // Assert number of variables
  ASSERT(xx.variables().size() == 1);
  const oops::Variable var = xx.variables()[0];

  // Global state
  auto globalState = geom.functionSpace().createField<double>(atlas::option::name(var.name())
    | atlas::option::levels(var.getLevels()) | atlas::option::global());

  // Gather data on main processor
  geom.functionSpace().gather(xx.fieldSet()[var.name()], globalState);

  // Get valid time components
  int year, month, day, hour, minute, second;
  xx.validTime().toYYYYMMDDhhmmss(year, month, day, hour, minute, second);

  // Get number of seconds since 00:00:00
  const double t = static_cast<double>(hour*3600+minute*60+second);

  if (comm_.rank() == 0) {
    // Get grid
    const atlas::StructuredGrid grid(geom.grid());

    // Create numpy array
    const int nx = (*params_)["nx"].cast<size_t>();
    const int ny = (*params_)["ny"].cast<size_t>();
    torch::Tensor stateTTensor = torch::empty({globalState.shape(1), ny, nx}, opts_);

    // Get view
    auto stateTView = stateTTensor.accessor<double, 3>();
    auto stateView = atlas::array::make_view<double, 2>(globalState);

    // Copy data to torch tensor
    int ix, iy;
    for (int jnode = 0; jnode < globalState.shape(0); ++jnode) {
      // Get X/Y indices
      grid.index2ij(jnode, ix, iy);
      for (int jlevel = 0; jlevel < globalState.shape(1); ++jlevel) {
        stateTView[jlevel][iy][ix] = stateView(jnode, jlevel);
      }
    }

    // Load python module
    const std::string moduleName = "PyTorchDDL95";
    pybind11::module_ exec = pybind11::module_::import(moduleName.c_str());

    // Execute time step
    pybind11::object result = exec.attr("step")(*params_, lonTTensor_, latTTensor_, cMaskTTensor_,
      yMaskTTensor_, t, stateTTensor);

    // Copy data from torch tensor
    for (int jnode = 0; jnode < globalState.shape(0); ++jnode) {
      grid.index2ij(jnode, ix, iy);
      for (int jlevel = 0; jlevel < globalState.shape(1); ++jlevel) {
        stateView(jnode, jlevel) = stateTView[jlevel][iy][ix];
      }
    }
  }

  // Scatter data from main processor
  geom.functionSpace().scatter(globalState, xx.fieldSet()[var.name()]);

  // Update valid time
  xx.updateTime(timeResolution_);

  oops::Log::trace() << classname() << "::step done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelPyTorchDDL95::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "PyTorchDDL95 model:" << std::endl;
  os << "- dt: " << timeResolution_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
