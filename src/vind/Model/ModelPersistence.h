/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <memory>
#include <ostream>
#include <string>

#include "eckit/config/Configuration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/memory/NonCopyable.h"

#include "oops/interface/ModelBase.h"
#include "oops/util/Duration.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vind/Traits.h"

namespace vind {
  class Geometry;
  class ModelAuxControl;
  class Fields;
  class State;

// -----------------------------------------------------------------------------
///  ModelPersistence class

class ModelPersistence: public oops::interface::ModelBase<Traits>,
                        private util::ObjectCounter<ModelPersistence> {
 public:
  static const std::string classname()
    {return "vind::ModelPersistence";}

/// OOPS interface

// Constructors/destructor
  ModelPersistence(const Geometry &,
                   const eckit::Configuration &);
  ModelPersistence(const ModelPersistence &);
  ~ModelPersistence()
    {}

// Prepare model integration
  void initialize(State &) const
    {}

// Model integration
  void step(State &,
            const ModelAuxControl &) const;
  int saveTrajectory(State &,
                     const ModelAuxControl &) const
    {throw eckit::NotImplemented(Here()); return 0;}

// Finish model integration
  void finalize(State &) const
    {}

// Utilities
  const util::Duration & timeResolution() const
    {return timeResolution_;}

 private:
  void print(std::ostream &) const
    {throw eckit::NotImplemented(Here());}

  const util::Duration timeResolution_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
