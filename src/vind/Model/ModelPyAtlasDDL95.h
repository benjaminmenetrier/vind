/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <pybind11/embed.h>

#include <memory>
#include <ostream>
#include <string>

#include "atlas/field.h"

#include "eckit/mpi/Comm.h"

#include "oops/util/ObjectCounter.h"

#include "vind/Model/ModelBase.h"
#include "vind/Python/PythonInterpreter.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class ModelAuxControl;
  class State;

// -----------------------------------------------------------------------------
/// ModelPyAtlasDDL95 class

class __attribute__((visibility("hidden"))) ModelPyAtlasDDL95:
  public ModelBase,
  private util::ObjectCounter<ModelPyAtlasDDL95> {
 public:
  static const std::string classname() {return "vind::ModelPyAtlasDDL95";}

  // Constructor/destructor
  ModelPyAtlasDDL95(const Geometry &,
                  const eckit::Configuration &);
  ~ModelPyAtlasDDL95();

  // Prepare model integration
  void initialize(State &) const override
    {}

  // Model integration
  void step(State &,
            const ModelAuxControl &) const override;

  // Finish model integration
  void finalize(State &) const override
    {}

  // Utilities
  const util::Duration & timeResolution() const
    {return timeResolution_;}

 private:
  void print(std::ostream &) const override;

  const util::Duration timeResolution_;
  const eckit::mpi::Comm & comm_;
  const PythonInterpreter pythonInterpreter_;
  std::unique_ptr<pybind11::dict> params_;
  atlas::FieldSet geom_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
