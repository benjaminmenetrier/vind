/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "oops/util/Duration.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vind/LinearModel/LinearModelBase.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class Increment;
  class ModelAuxControl;
  class ModelAuxIncrement;
  class State;

// -----------------------------------------------------------------------------
///  LinearModel class

class LinearModel: public util::Printable,
                   private util::ObjectCounter<LinearModel> {
 public:
  static const std::string classname()
    {return "vind::LinearModel";}
  static std::vector<std::string> names()
    {return {"DDL95", "NumPyDDL95", "PyTorchDDL95"};}

  // Constructors/destructor
  LinearModel(const Geometry &,
              const eckit::Configuration &);
  ~LinearModel() = default;

  // Set the linearization trajectory
  void setTrajectory(const State &,
                     State &,
                     const ModelAuxControl &);

  // Prepare TL model integration
  void initializeTL(Increment &) const;

  // TL model integration
  void stepTL(Increment &,
              const ModelAuxIncrement &) const;

  // Finish TL model integration
  void finalizeTL(Increment &) const;

  // Prepare AD model integration
  void initializeAD(Increment &) const;

  // Prepare AD model integration
  void stepAD(Increment &,
              const ModelAuxIncrement &) const;

  // Finish AD model integration
  void finalizeAD(Increment &) const;

  // Utilities
  const util::Duration & timeResolution() const
    {return linearModel_->timeResolution();}
  const util::Duration & stepTrajectory() const
    {return linearModel_->stepTrajectory();}

 private:
  void print(std::ostream &) const;

  std::unique_ptr<LinearModelBase> linearModel_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
