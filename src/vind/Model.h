/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <memory>
#include <ostream>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/NonCopyable.h"

#include "oops/util/Duration.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class ModelAuxControl;
  class Fields;
  class State;

// -----------------------------------------------------------------------------
///  Model class

class Model: public util::Printable,
             private eckit::NonCopyable,
             private util::ObjectCounter<Model> {
 public:
  static const std::string classname()
    {return "vind::Model";}

/// OOPS interface

// Constructors/destructor
  Model(const Geometry &,
        const eckit::Configuration &);
  Model(const Model &);
  ~Model()
    {}

// Prepare model integration
  void initialize(State &,
                  const ModelAuxControl &) const
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
