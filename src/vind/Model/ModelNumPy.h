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

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class ModelAuxControl;
  class State;

// -----------------------------------------------------------------------------
///  ModelNumPy class

class __attribute__((visibility("hidden"))) ModelNumPy:
  public ModelBase,
  private util::ObjectCounter<ModelNumPy> {
 public:
  static const std::string classname() {return "vind::ModelNumPy";}

  // Constructor/destructor
  ModelNumPy(const Geometry &,
             const eckit::Configuration &);
  ~ModelNumPy();

  // Prepare model integration
  void initialize(State &) const override;

  // Model integration
  void step(State &,
            const ModelAuxControl &) const override;

  // Finish model integration
  void finalize(State &) const override;

  // Utilities
  const util::Duration & timeResolution() const
    {return timeResolution_;}

 private:
  void print(std::ostream &) const override;

  const util::Duration timeResolution_;
  const eckit::mpi::Comm & comm_;
  const std::string pythonModule_;
  std::string pythonDir_;
  std::unique_ptr<pybind11::dict> initData_;
  std::unique_ptr<pybind11::object> model_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
