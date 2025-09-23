/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <pybind11/embed.h>

#include <memory>
#include <ostream>
#include <string>

#include "eckit/mpi/Comm.h"

#include "oops/util/ObjectCounter.h"

#include "vind/Model/ModelBase.h"

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

 private:
  void print(std::ostream &) const override;

  const eckit::mpi::Comm & comm_;
  const std::string pythonModule_;
  std::string pythonDir_;
  std::unique_ptr<pybind11::dict> initData_;
  std::unique_ptr<pybind11::object> model_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
