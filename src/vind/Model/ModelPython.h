/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <ostream>
#include <string>

#include <pybind11/embed.h>

#include "oops/base/Variables.h"
#include "oops/util/Duration.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vind/Model/ModelBase.h"

namespace py = pybind11;

// Forward declarations
namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class ModelAuxControl;
  class Increment;
  class State;

// -----------------------------------------------------------------------------
///  ModelPython class

class __attribute__((visibility("hidden"))) ModelPython: public ModelBase,
                                                         private util::ObjectCounter<ModelPython> {
 public:
  static const std::string classname() {return "vind::ModelPython";}

  ModelPython(const Geometry &,
              const eckit::Configuration &);
  ~ModelPython();

/// Prepare model integration
  void initialize(State &) const override;

/// Model integration
  void step(State &,
            const ModelAuxControl &) const override;

/// Finish model integration
  void finalize(State &) const override;

/// Utilities
  const util::Duration & timeResolution() const override
    {return timeResolution_;}

 private:
  void print(std::ostream &) const override;

  const eckit::mpi::Comm & comm_;
  const util::Duration timeResolution_;
  const std::string pythonModule_;
  std::string pythonDir_;
  std::unique_ptr<py::dict> initData_;
  std::unique_ptr<py::object> model_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
