/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <cmath>
#include <ostream>
#include <string>

#include "oops/util/ObjectCounter.h"

#include "vind/Model/ModelBase.h"

namespace vind {
  class Fields;
  class Geometry;
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

 private:
  void print(std::ostream &) const override;
  void tendency(const Fields &,
                Fields &) const;

  const util::Duration timeResolution_;
  const double F_ = 8.0;
  const double omega_ = 2.0*M_PI/(24.0*3600.0);
  const double nu_ = 1.0;
  const double dti_sub_ = 0.02;
  size_t nx_;
  size_t ny_;
  std::vector<double> x_;
  std::vector<double> y_;
  size_t nsub_;
  util::Duration dt_sub_;
  util::Duration dt_sub_half_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
