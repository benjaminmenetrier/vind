/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "oops/util/Duration.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vind/Model/ModelBase.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class ModelAuxControl;
  class State;

// -----------------------------------------------------------------------------
///  Model class

class Model: public util::Printable,
             private util::ObjectCounter<Model> {
 public:
  static const std::string classname()
    {return "vind::Model";}
  static std::vector<std::string> names()
    {return {"DDL95", "python"};}

  // Constructors/destructor
  Model(const Geometry &,
        const eckit::Configuration &);
  ~Model() = default;

  // Prepare model integration
  void initialize(State &) const;

  // Model integration
  void step(State &,
            const ModelAuxControl &) const;

  // Finish model integration
  void finalize(State &) const;

  // Utilities
  const util::Duration & timeResolution() const
    {return model_->timeResolution();}

 private:
  void print(std::ostream &) const;

  std::unique_ptr<ModelBase> model_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
