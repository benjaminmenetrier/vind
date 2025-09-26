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
  comm_(geom.getComm()),
  pythonModule_(config.getString("python module")) {
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
  (*initData_)["time step"] = timeResolution_.toSeconds();

  // Add module-specific parameters
  if (pythonModule_ == "ModelPythonDDL95") {
    // Check geometry
    ASSERT(geom.gridType() == "regional");

    // Get dimensions
    const atlas::RegularGrid & grid = geom.grid();
    (*initData_)["nx"] = grid.nx();
    (*initData_)["ny"] = grid.ny();
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

  // Global state
  atlas::FieldSet globalState;
  for (const auto & var : xx.variables()) {
    auto glbField = fs.createField<double>(atlas::option::name(var.name())
      | atlas::option::levels(var.getLevels()) | atlas::option::global());
    globalState.add(glbField);
  }

  // Gather data on main processor
  fs.gather(xx.fieldSet(), globalState);

  if (comm_.rank() == 0) {
    // Create data dictionary
    auto stepData = pybind11::dict();

    // Add date/time in data dictionary
    int year, month, day, hour, minute, second;
    xx.validTime().toYYYYMMDDhhmmss(year, month, day, hour, minute, second);
    stepData["year"] = year;
    stepData["month"] = month;
    stepData["day"] = day;
    stepData["hour"] = hour;
    stepData["minute"] = minute;
    stepData["second"] = second;

    // Add fields in data dictionary
    auto stateData = pybind11::dict();
    for (const auto & var : xx.variables()) {
      auto field = globalState[var.name()];
      if (pythonModule_ == "ModelPythonDDL95") {
        // Create numpy array
        const int nx = (*initData_)["nx"].cast<size_t>();
        const int ny = (*initData_)["ny"].cast<size_t>();
        pybind11::array_t<double> dataNp({field.shape(1), ny, nx});

        // Copy data to numpy array
        auto dataView = dataNp.mutable_unchecked<3>();
        const auto view = atlas::array::make_view<double, 2>(field);
        const atlas::StructuredGrid grid(geom.grid());
        int ix, iy;
        for (int jnode = 0; jnode < field.shape(0); ++jnode) {
          // Get X/Y indices
          grid.index2ij(jnode, ix, iy);
          for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
            dataView(jlevel, iy, ix) = view(jnode, jlevel);
          }
        }

        // Add numpy array to state dictionary
        stateData[var.name().c_str()] = dataNp;
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }
    stepData["state"] = stateData;

    // Load python module
    pybind11::module_ exec = pybind11::module_::import(pythonModule_.c_str());

    // Execute time step
    pybind11::object result = exec.attr("step")(*model_, stepData);

    for (const auto & var : xx.variables()) {
      auto field = globalState[var.name()];
      if (pythonModule_ == "ModelPythonDDL95") {
        // Copy data from numpy array
        const auto dataView = stepData["state"][var.name().c_str()].cast<pybind11::array_t<double>>()
          .unchecked<3>();
        auto view = atlas::array::make_view<double, 2>(field);
        const atlas::StructuredGrid grid(geom.grid());
        int ix, iy;
        for (int jnode = 0; jnode < field.shape(0); ++jnode) {
          grid.index2ij(jnode, ix, iy);
          for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
            view(jnode, jlevel) = dataView(jlevel, iy, ix);
          }
        }
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }
  }

  // Scatter data from main processor
  fs.scatter(globalState, xx.fieldSet());

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
