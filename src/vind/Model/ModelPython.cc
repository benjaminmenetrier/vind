/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/Model/ModelPython.h"

#include <filesystem>
#include <pybind11/numpy.h>

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"
#include "vind/State.h"

namespace vind {

// -----------------------------------------------------------------------------

static ModelMaker<ModelPython> makerPython_("python");

// -----------------------------------------------------------------------------

ModelPython::ModelPython(const Geometry & geom,
                         const eckit::Configuration & config)
  : timeResolution_(config.getString("time step")),
  comm_(geom.getComm()), pythonModule_(config.getString("python module")) {
  oops::Log::trace() << classname() << "::ModelPython starting" << std::endl;

  // Get executable directory
#if defined(_MSC_VER)
  wchar_t path[FILENAME_MAX] = { 0 };
  GetModuleFileNameW(nullptr, path, FILENAME_MAX);
  pythonDir_ = std::filesystem::path(path).parent_path().string();
#else
  char path[FILENAME_MAX];
  ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
  pythonDir_ = std::filesystem::path(std::string(path, (count > 0) ? count : 0))
    .parent_path().string();
#endif

  // Intialize pybind11 interpreter
  pybind11::initialize_interpreter();

  // Create data dictionary
  initData_.reset(new pybind11::dict());

  // Add time-step
  (*initData_)["tstep"] = timeResolution_.toSeconds();

  // Number of threads
  (*initData_)["threads"] = 1;

  // Add module-specific parameters
  if (pythonModule_ == "ModelPythonPyQG") {
    // Check geometry
    ASSERT((geom.gridType() == "structured") || (geom.gridType() == "regular_lonlat")
      || (geom.gridType() == "zonal_band"));
    const atlas::RegularGrid & grid = geom.grid();
    const size_t nx = grid.nx();
    const size_t ny = grid.ny();
    ASSERT(nx == ny);

    // Add grid size
    (*initData_)["nx"] = nx;
  }

  // Insert python module into PYTHONPATH
  pybind11::module_ sys = pybind11::module_::import("sys");
  sys.attr("path").attr("insert")(1, pythonDir_.c_str());

  // Load python module
  pybind11::module_ exec = pybind11::module_::import(pythonModule_.c_str());

  if (comm_.rank() == 0) {
    // Execute initialization
    model_.reset(new pybind11::object(exec.attr("constructor")(*initData_)));
  }

  oops::Log::trace() << classname() << "::ModelPython done" << std::endl;
}

// -----------------------------------------------------------------------------

ModelPython::~ModelPython() {
  oops::Log::trace() << classname() << "::~ModelPython starting" << std::endl;

  // Destroy initialization data
  initData_.reset();

  // Destroy model
  model_.reset();

  // Finalize pybind11 interpreter
  pybind11::finalize_interpreter();

  oops::Log::trace() << classname() << "::~ModelPython done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelPython::initialize(State & xx) const {
  oops::Log::trace() << classname() << "::initialize starting" << std::endl;

  // Add initalization here if needed

  oops::Log::trace() << classname() << "::initialize done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelPython::step(State & xx,
                       const ModelAuxControl & xxAux) const {
  oops::Log::trace() << classname() << "::step starting" << std::endl;

  // Get geometry
  const Geometry & geom(xx.fields().geometry());

  // Get function space
  const atlas::FunctionSpace fs(geom.functionSpace());

  // Global data
  atlas::FieldSet globalData;
  for (const auto & var : xx.variables()) {
    atlas::Field glbField = fs.createField<double>(atlas::option::name(var.name())
      | atlas::option::levels(var.getLevels()) | atlas::option::global());
    globalData.add(glbField);
  }

  // Gather data to main processor
  fs.gather(xx.fieldSet(), globalData);

  if (comm_.rank() == 0) {
    // Create data dictionary
    auto stepData = pybind11::dict();

    for (const auto & var : xx.variables()) {
      atlas::Field field = globalData[var.name()];
      if (pythonModule_ == "ModelPythonPyQG") {
        // Create numpy array
        const int nx = (*initData_)["nx"].cast<size_t>();
        pybind11::array_t<double> dataNp({field.shape(1), nx, nx});

        // Copy data to numpy array
        auto dataView = dataNp.mutable_unchecked<3>();
        const auto view = atlas::array::make_view<double, 2>(field);
        const atlas::StructuredGrid grid(geom.grid());
        int i, j;
        for (atlas::idx_t jnode = 0; jnode < field.shape(0); ++jnode) {
          grid.index2ij(jnode, i, j);
          for (atlas::idx_t jlevel = 0; jlevel < field.shape(1); ++jlevel) {
            dataView(jlevel, j, i) = view(jnode, jlevel);
          }
        }

        // Add numpy array to data directory
        stepData[var.name().c_str()] = dataNp;
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }

    // Load python module
    pybind11::module_ exec = pybind11::module_::import(pythonModule_.c_str());

    // Execute time step
    pybind11::object result = exec.attr("step")(*model_, stepData);

    for (const auto & var : xx.variables()) {
      atlas::Field field = globalData[var.name()];
      if (pythonModule_ == "ModelPythonPyQG") {
        // Copy data from numpy array
        const auto dataView = stepData[var.name().c_str()].cast<pybind11::array_t<double>>()
          .unchecked<3>();
        auto view = atlas::array::make_view<double, 2>(field);
        const atlas::StructuredGrid grid(geom.grid());
        int i, j;
        for (atlas::idx_t jnode = 0; jnode < field.shape(0); ++jnode) {
          grid.index2ij(jnode, i, j);
          for (atlas::idx_t jlevel = 0; jlevel < field.shape(1); ++jlevel) {
            view(jnode, jlevel) = dataView(jlevel, j, i);
          }
        }
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }
  }

  // Scatter data from main processor
  fs.scatter(globalData, xx.fieldSet());

  // Update valid time
  xx.updateTime(timeResolution_);

  oops::Log::trace() << classname() << "::step done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelPython::finalize(State & xx) const {
  oops::Log::trace() << classname() << "::finalize starting" << std::endl;

  // Add finalization here if needed

  oops::Log::trace() << classname() << "::finalize done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelPython::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "Python model:" << std::endl;
  os << "- dt = " << timeResolution_ << std::endl;
  os << "- python module = " << pythonModule_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
