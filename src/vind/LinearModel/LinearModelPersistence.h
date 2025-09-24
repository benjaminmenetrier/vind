/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <ostream>
#include <string>

#include "oops/util/Duration.h"
#include "oops/util/ObjectCounter.h"

#include "vind/LinearModel/LinearModelBase.h"

namespace vind {
  class Geometry;
  class Increment;
  class ModelAuxControl;
  class ModelAuxIncrement;

// -----------------------------------------------------------------------------
///  LinearModelPersistence class

class LinearModelPersistence: public LinearModelBase,
                              private util::ObjectCounter<LinearModelPersistence> {
 public:
  static const std::string classname() {return "vind::LinearModelPersistence";}

  LinearModelPersistence(const Geometry &,
                         const eckit::Configuration &);
  ~LinearModelPersistence()
    {}

  // Prepare TL model integration
  void initializeTL(Increment &) const override
    {}

  // TL model integration
  void stepTL(Increment &,
              const ModelAuxIncrement &) const override;

  // Finish TL model integration
  void finalizeTL(Increment &) const override
    {}

  // Prepare AD model integration
  void initializeAD(Increment &) const override
    {}

  // AD model integration
  void stepAD(Increment &,
              const ModelAuxIncrement &) const override;

  // Finish AD model integration
  void finalizeAD(Increment &) const override
    {}

  // Utilities
  const util::Duration & timeResolution() const
    {return timeResolution_;}
  const util::Duration & stepTrajectory() const
    {return stepTrajectory_;}

 private:
  void print(std::ostream &) const override;

  const util::Duration timeResolution_;
  const util::Duration stepTrajectory_;
  const double persistenceFactor_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
