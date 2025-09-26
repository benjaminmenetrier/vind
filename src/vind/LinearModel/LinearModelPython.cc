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
  (*initData_)["time step"] = timeResolution_.toSeconds();

  // Add module-specific parameters
  if (pythonModule_ == "ModelPythonDDL95") {
    // Check geometry
    ASSERT(geom.gridType() == "regional");
    const atlas::RegularGrid & grid = geom.grid();
    const size_t nx = grid.nx();
    const size_t ny = grid.ny();

    // Add grid size
    (*initData_)["nx"] = nx;
    (*initData_)["ny"] = ny;
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

  // Global trajectory
  atlas::FieldSet globalTraj;
  for (const auto & var : dx.variables()) {
    atlas::Field glbField = fs.createField<double>(atlas::option::name(var.name())
      | atlas::option::levels(var.getLevels()) | atlas::option::global());
    globalTraj.add(glbField);
  }

  // Global increment
  atlas::FieldSet globalIncr;
  for (const auto & var : dx.variables()) {
    atlas::Field glbField = fs.createField<double>(atlas::option::name(var.name())
      | atlas::option::levels(var.getLevels()) | atlas::option::global());
    globalIncr.add(glbField);
  }

  // Gather data on main processor
  fs.gather(traj_.at(dx.validTime()).fieldSet(), globalTraj);
  fs.gather(dx.fieldSet(), globalIncr);

  if (comm_.rank() == 0) {
    // Create data dictionary
    auto stepData = pybind11::dict();

    // Add date/time in data dictionary
    int year, month, day, hour, minute, second;
    dx.validTime().toYYYYMMDDhhmmss(year, month, day, hour, minute, second);
    stepData["year"] = year;
    stepData["month"] = month;
    stepData["day"] = day;
    stepData["hour"] = hour;
    stepData["minute"] = minute;
    stepData["second"] = second;

    // Add trajectory in data dictionary
    auto trajData = pybind11::dict();
    for (const auto & var : dx.variables()) {
      atlas::Field field = globalIncr[var.name()];
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
          grid.index2ij(jnode, ix, iy);
          for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
            dataView(jlevel, iy, ix) = view(jnode, jlevel);
          }
        }

        // Add numpy array to trajectory dictionary
        trajData[var.name().c_str()] = dataNp;
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }
    stepData["trajectory"] = trajData;

    // Add increment in data dictionary
    auto incrData = pybind11::dict();
    for (const auto & var : dx.variables()) {
      atlas::Field field = globalIncr[var.name()];
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
          grid.index2ij(jnode, ix, iy);
          for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
            dataView(jlevel, iy, ix) = view(jnode, jlevel);
          }
        }

        // Add numpy array to increment dictionary
        incrData[var.name().c_str()] = dataNp;
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }
    stepData["increment"] = incrData;

    // Load python module
    pybind11::module_ exec = pybind11::module_::import(pythonModule_.c_str());

    // Execute time step
    pybind11::object result = exec.attr("stepTL")(*linearModel_, stepData);

    for (const auto & var : dx.variables()) {
      atlas::Field field = globalIncr[var.name()];
      if (pythonModule_ == "ModelPythonDDL95") {
        // Copy data from numpy array
        const auto dataView = stepData["increment"][var.name().c_str()].cast<pybind11::array_t<double>>()
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
  fs.scatter(globalIncr, dx.fieldSet());

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

  // Global trajectory
  atlas::FieldSet globalTraj;
  for (const auto & var : dx.variables()) {
    atlas::Field glbField = fs.createField<double>(atlas::option::name(var.name())
      | atlas::option::levels(var.getLevels()) | atlas::option::global());
    globalTraj.add(glbField);
  }

  // Global increment
  atlas::FieldSet globalIncr;
  for (const auto & var : dx.variables()) {
    atlas::Field glbField = fs.createField<double>(atlas::option::name(var.name())
      | atlas::option::levels(var.getLevels()) | atlas::option::global());
    globalIncr.add(glbField);
  }

  // Gather data on main processor
  fs.gather(traj_.at(dx.validTime()).fieldSet(), globalTraj);
  fs.gather(dx.fieldSet(), globalIncr);

  if (comm_.rank() == 0) {
    // Create data dictionary
    auto stepData = pybind11::dict();

    // Add date/time in data dictionary
    int year, month, day, hour, minute, second;
    dx.validTime().toYYYYMMDDhhmmss(year, month, day, hour, minute, second);
    stepData["year"] = year;
    stepData["month"] = month;
    stepData["day"] = day;
    stepData["hour"] = hour;
    stepData["minute"] = minute;
    stepData["second"] = second;

    // Add trajectory in data dictionary
    auto trajData = pybind11::dict();
    for (const auto & var : dx.variables()) {
      atlas::Field field = globalIncr[var.name()];
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
          grid.index2ij(jnode, ix, iy);
          for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
            dataView(jlevel, iy, ix) = view(jnode, jlevel);
          }
        }

        // Add numpy array to trajectory dictionary
        trajData[var.name().c_str()] = dataNp;
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }
    stepData["trajectory"] = trajData;

    // Add increment in data dictionary
    auto incrData = pybind11::dict();
    for (const auto & var : dx.variables()) {
      atlas::Field field = globalIncr[var.name()];
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
          grid.index2ij(jnode, ix, iy);
          for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
            dataView(jlevel, iy, ix) = view(jnode, jlevel);
          }
        }

        // Add numpy array to increment dictionary
        incrData[var.name().c_str()] = dataNp;
      } else {
        throw eckit::UserError("wrong python module", Here());
      }
    }
    stepData["increment"] = incrData;

    // Load python module
    pybind11::module_ exec = pybind11::module_::import(pythonModule_.c_str());

    // Execute time step
    pybind11::object result = exec.attr("stepAD")(*linearModel_, stepData);

    for (const auto & var : dx.variables()) {
      atlas::Field field = globalIncr[var.name()];
      if (pythonModule_ == "ModelPythonDDL95") {
        // Copy data from numpy array
        const auto dataView = stepData["increment"][var.name().c_str()].cast<pybind11::array_t<double>>()
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
  fs.scatter(globalIncr, dx.fieldSet());

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
