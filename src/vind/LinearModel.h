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

#include "eckit/config/Configuration.h"
#include "eckit/exception/Exceptions.h"

#include "oops/util/Duration.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vind/Traits.h"

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
    {return {"vind"};}

/// OOPS interface

// Constructors/destructor
  LinearModel(const Geometry &,
              const eckit::Configuration &)
    {throw eckit::NotImplemented(Here());}
  ~LinearModel()
    {}

// Set the linearization trajectory
  void setTrajectory(const State &,
                     State &,
                     const ModelAuxControl &)
    {throw eckit::NotImplemented(Here());}

// TL forecast
  void initializeTL(Increment &) const
    {throw eckit::NotImplemented(Here());}
  void stepTL(Increment &,
              const ModelAuxIncrement &) const
    {throw eckit::NotImplemented(Here());}
  void finalizeTL(Increment &) const
    {throw eckit::NotImplemented(Here());}

// AD forecast
  void initializeAD(Increment &) const
    {throw eckit::NotImplemented(Here());}
  void stepAD(Increment &,
              ModelAuxIncrement &) const
    {throw eckit::NotImplemented(Here());}
  void finalizeAD(Increment &) const
    {throw eckit::NotImplemented(Here());}

// Information and diagnostics
  const util::Duration & timeResolution() const
    {return timeResolution_;}
  const util::Duration & stepTrajectory() const
    {return stepTraj_;}

 private:
  void print(std::ostream &) const
    {throw eckit::NotImplemented(Here());}

  const util::Duration timeResolution_;
  const util::Duration stepTraj_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
