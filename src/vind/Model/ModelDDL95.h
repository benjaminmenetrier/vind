/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <cmath>
#include <ostream>
#include <string>

#include "atlas/field.h"

#include "oops/util/ObjectCounter.h"

#include "vind/Model/ModelBase.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Fields;
  class Geometry;
  class Increment;
  class ModelAuxControl;
  class State;

// -----------------------------------------------------------------------------
///  ModelDDL95 class

class ModelDDL95: public ModelBase,
                  private util::ObjectCounter<ModelDDL95> {
 public:
  static const std::string classname() {return "vind::ModelDDL95";}

  // Constructor/destructor
  ModelDDL95(const Geometry &,
             const eckit::Configuration &);
  ~ModelDDL95()
    {}

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

  // Tendency
  void tendency(const State &,
                Increment &) const;

 private:
  void print(std::ostream &) const override;

  const util::Duration timeResolution_;
  const double F_ = 8.0;
  const double omega_ = 2.0*M_PI/(24.0*3600.0);
  const double nu_ = 1.0;
  size_t nx_;
  size_t ny_;
  size_t ixMin_;
  size_t ixMax_;
  size_t iyMin_;
  size_t iyMax_;
  atlas::Field lonLatField_;
  double dti_;
  util::Duration dt_half_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
