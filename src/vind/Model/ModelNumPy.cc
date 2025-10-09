/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/Model/ModelNumPy.h"

#include <filesystem>
#include <pybind11/numpy.h>

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"
#include "vind/State.h"

namespace vind {

// -----------------------------------------------------------------------------

static ModelMaker<ModelNumPy> makerPython_("NumPy");

// -----------------------------------------------------------------------------

ModelNumPy::ModelNumPy(const Geometry & geom,
                       const eckit::Configuration & config)
  : timeResolution_(config.getString("time step")),
  comm_(geom.getComm()),
  pythonModule_(config.getString("python module")) {
  oops::Log::trace() << classname() << "::ModelNumPy starting" << std::endl;

  // Get executable directory
#if defined(_MSC_VER)
  wchar_t path[FILENAME_MAX] = {0};
  GetModuleFileNameW(nullptr, path, FILENAME_MAX);
  pythonDir_ = std::filesystem::path(path).parent_path().string();
#else
  char path[FILENAME_MAX];
  ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
  pythonDir_ = std::filesystem::path(std::string(path, (count > 0) ? count : 0))
    .parent_path().string();
#endif

  // Create data dictionary
  initData_.reset(new pybind11::dict());

  // Add time-step
  (*initData_)["time step"] = timeResolution_.toSeconds();

  // Add module-specific parameters
  if (pythonModule_ == "DDL95") {
    // Get dimensions
    const atlas::RegularGrid & grid = geom.grid();
    const int nx = grid.nx();
    const int ny = grid.ny();
    (*initData_)["nx"] = grid.nx();
    (*initData_)["ny"] = grid.ny();

    // Get computation zone boundaries
    if (grid.periodic()) {
      (*initData_)["ixMin"] = 0;
      (*initData_)["ixMax"] = nx-1;
    } else {
      (*initData_)["ixMin"] = 2;
      (*initData_)["ixMax"] = nx-2;
    }
    if (geom.gridType() == "regional") {
      (*initData_)["iyMin"] = 0;
      (*initData_)["iyMax"] = ny-1;
    } else {
      (*initData_)["iyMin"] = 1;
      (*initData_)["iyMax"] = ny-2;
    }

    // Define x/y coordinates
    pybind11::array_t<double> lonNp({ny, nx});
    pybind11::array_t<double> latNp({ny, nx});
    auto lonView = lonNp.mutable_unchecked<2>();
    auto latView = latNp.mutable_unchecked<2>();
    const double deg2rad = M_PI/180.0;
    for (int jx = 0; jx < nx; ++jx) {
      for (int jy = 0; jy < ny; ++jy) {
        const atlas::PointLonLat pointLonLat = grid.lonlat(jx, jy);
        lonView(jy, jx) = pointLonLat.lon()*deg2rad;
        latView(jy, jx) = pointLonLat.lat()*deg2rad;
      }
    }
    (*initData_)["lon"] = lonNp;
    (*initData_)["lat"] = latNp;
  }

  // Insert python module into PYTHONPATH
  pybind11::module_ sys = pybind11::module_::import("sys");
  sys.attr("path").attr("insert")(1, pythonDir_.c_str());

  // Load python module
  const std::string moduleName = "NumPy" + pythonModule_;
  pybind11::module_ exec = pybind11::module_::import(moduleName.c_str());

  if (comm_.rank() == 0) {
    // Execute initialization
    model_.reset(new pybind11::object(exec.attr("constructor")(*initData_)));
  }

  oops::Log::trace() << classname() << "::ModelNumPy done" << std::endl;
}

// -----------------------------------------------------------------------------

ModelNumPy::~ModelNumPy() {
  oops::Log::trace() << classname() << "::~ModelNumPy starting" << std::endl;

  // Destroy initialization data
  initData_.reset();

  // Destroy model
  model_.reset();

  oops::Log::trace() << classname() << "::~ModelNumPy done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelNumPy::initialize(State & xx) const {
  oops::Log::trace() << classname() << "::initialize starting" << std::endl;

  // Add initalization here if needed

  oops::Log::trace() << classname() << "::initialize done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelNumPy::step(State & xx,
                      const ModelAuxControl & xxAux) const {
  oops::Log::trace() << classname() << "::step starting" << std::endl;

  // Get geometry
  const Geometry & geom(xx.geometry());

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
      if (pythonModule_ == "DDL95") {
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
    const std::string moduleName = "NumPy" + pythonModule_;
    pybind11::module_ exec = pybind11::module_::import(moduleName.c_str());

    // Execute time step
    pybind11::object result = exec.attr("step")(*model_, stepData);

    for (const auto & var : xx.variables()) {
      auto field = globalState[var.name()];
      if (pythonModule_ == "DDL95") {
        // Copy data from numpy array
        const auto dataView = stepData["state"][var.name().c_str()]
          .cast<pybind11::array_t<double>>().unchecked<3>();
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

void ModelNumPy::finalize(State & xx) const {
  oops::Log::trace() << classname() << "::finalize starting" << std::endl;

  // Add finalization here if needed

  oops::Log::trace() << classname() << "::finalize done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelNumPy::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "NumPy model:" << std::endl;
  os << "- dt = " << timeResolution_ << std::endl;
  os << "- python module = " << pythonModule_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
