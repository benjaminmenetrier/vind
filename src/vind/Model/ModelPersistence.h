/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <ostream>
#include <string>

#include "oops/base/Variables.h"
#include "oops/util/Duration.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

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
///  ModelPersistence class

class ModelPersistence: public ModelBase,
                        private util::ObjectCounter<ModelPersistence> {
 public:
  static const std::string classname() {return "vind::ModelPersistence";}

  ModelPersistence(const Geometry &,
                   const eckit::Configuration &);
  ~ModelPersistence()
    {}

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

  const util::Duration timeResolution_;
  const double persistenceFactor_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
