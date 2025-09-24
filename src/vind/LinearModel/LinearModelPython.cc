/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/LinearModel/LinearModelPython.h"

#include <filesystem>
#include <pybind11/numpy.h>

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"
#include "vind/Increment.h"

namespace vind {

// -----------------------------------------------------------------------------

static LinearModelMaker<LinearModelPython> makerPython_("python");

// -----------------------------------------------------------------------------

LinearModelPython::LinearModelPython(const Geometry & geom,
                                     const eckit::Configuration & config)
  : timeResolution_(config.getString("time step")),
  stepTrajectory_(config.getString("trajectory step")),
  comm_(geom.getComm()),
  pythonModule_(config.getString("python module")) {
  oops::Log::trace() << classname() << "::LinearModelPython starting" << std::endl;

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
  if (pythonModule_ == "YOUR_PYTHON_MODULE") {
    // Call your python module
  }

  // Insert python module into PYTHONPATH
  pybind11::module_ sys = pybind11::module_::import("sys");
  sys.attr("path").attr("insert")(1, pythonDir_.c_str());

  // Load python module
  pybind11::module_ exec = pybind11::module_::import(pythonModule_.c_str());

  if (comm_.rank() == 0) {
    // Execute initialization
    linearModel_.reset(new pybind11::object(exec.attr("constructor")(*initData_)));
  }

  oops::Log::trace() << classname() << "::LinearModelPython done" << std::endl;
}

// -----------------------------------------------------------------------------

LinearModelPython::~LinearModelPython() {
  oops::Log::trace() << classname() << "::~LinearModelPython starting" << std::endl;

  // Destroy initialization data
  initData_.reset();

  // Destroy model
  linearModel_.reset();

  // Finalize pybind11 interpreter
  pybind11::finalize_interpreter();

  oops::Log::trace() << classname() << "::~LinearModelPython done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPython::initializeTL(Increment & dx) const {
  oops::Log::trace() << classname() << "::initializeTL starting" << std::endl;

  // Add initalization here if needed

  oops::Log::trace() << classname() << "::initializeTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPython::stepTL(Increment & dx,
                               const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepTL starting" << std::endl;

  // Get geometry
  const Geometry & geom(dx.fields().geometry());

  // Get function space
  const atlas::FunctionSpace fs(geom.functionSpace());

  // Global data
  atlas::FieldSet globalData;
  for (const auto & var : dx.variables()) {
    atlas::Field glbField = fs.createField<double>(atlas::option::name(var.name())
      | atlas::option::levels(var.getLevels()) | atlas::option::global());
    globalData.add(glbField);
  }

  // Gather data to main processor
  fs.gather(dx.fieldSet(), globalData);

  if (comm_.rank() == 0) {
    // Create data dictionary
    auto stepData = pybind11::dict();

    for (const auto & var : dx.variables()) {
      atlas::Field field = globalData[var.name()];
      if (pythonModule_ == "YOUR_PYTHON_MODULE") {
        // Call your python module
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }

    // Load python module
    pybind11::module_ exec = pybind11::module_::import(pythonModule_.c_str());

    // Execute time step
    pybind11::object result = exec.attr("stepTL")(*linearModel_, stepData);

    for (const auto & var : dx.variables()) {
      atlas::Field field = globalData[var.name()];
      if (pythonModule_ == "YOUR_PYTHON_MODULE") {
        // Call your python module
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }
  }

  // Scatter data from main processor
  fs.scatter(globalData, dx.fieldSet());

  // Update valid time
  dx.updateTime(timeResolution_);

  oops::Log::trace() << classname() << "::stepTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPython::finalizeTL(Increment & dx) const {
  oops::Log::trace() << classname() << "::finalizeTL starting" << std::endl;

  // Add finalization here if needed

  oops::Log::trace() << classname() << "::finalizeTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPython::initializeAD(Increment & dx) const {
  oops::Log::trace() << classname() << "::initializeAD starting" << std::endl;

  // Add initalization here if needed

  oops::Log::trace() << classname() << "::initializeAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPython::stepAD(Increment & dx,
                               const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepAD starting" << std::endl;

  // Get geometry
  const Geometry & geom(dx.fields().geometry());

  // Get function space
  const atlas::FunctionSpace fs(geom.functionSpace());

  // Global data
  atlas::FieldSet globalData;
  for (const auto & var : dx.variables()) {
    atlas::Field glbField = fs.createField<double>(atlas::option::name(var.name())
      | atlas::option::levels(var.getLevels()) | atlas::option::global());
    globalData.add(glbField);
  }

  // Gather data to main processor
  fs.gather(dx.fieldSet(), globalData);

  if (comm_.rank() == 0) {
    // Create data dictionary
    auto stepData = pybind11::dict();

    for (const auto & var : dx.variables()) {
      atlas::Field field = globalData[var.name()];
      if (pythonModule_ == "YOUR_PYTHON_MODULE") {
        // Call your python module
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }

    // Load python module
    pybind11::module_ exec = pybind11::module_::import(pythonModule_.c_str());

    // Execute time step
    pybind11::object result = exec.attr("stepAD")(*linearModel_, stepData);

    for (const auto & var : dx.variables()) {
      atlas::Field field = globalData[var.name()];
      if (pythonModule_ == "YOUR_PYTHON_MODULE") {
        // Call your python module
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }
  }

  // Scatter data from main processor
  fs.scatter(globalData, dx.fieldSet());

  // Update valid time
  dx.updateTime(-timeResolution_);

  oops::Log::trace() << classname() << "::stepAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPython::finalizeAD(Increment & dx) const {
  oops::Log::trace() << classname() << "::finalizeAD starting" << std::endl;

  // Add finalization here if needed

  oops::Log::trace() << classname() << "::finalizeAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPython::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "Python linear model:" << std::endl;
  os << "- dt = " << timeResolution_ << std::endl;
  os << "- python module = " << pythonModule_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
