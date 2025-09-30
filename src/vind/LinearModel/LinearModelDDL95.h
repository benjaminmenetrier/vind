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
///  LinearModelDDL95 class

class LinearModelDDL95: public LinearModelBase,
                        private util::ObjectCounter<LinearModelDDL95> {
 public:
  static const std::string classname() {return "vind::LinearModelDDL95";}

  LinearModelDDL95(const Geometry &,
                   const eckit::Configuration &);
  ~LinearModelDDL95()
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
    {return dt_sub_half_;}

 private:
  void print(std::ostream &) const override;
  void tendencyTL(const Fields &,
                  Fields &) const;
  void tendencyAD(const Fields &,
                  Fields &) const;

  const util::Duration timeResolution_;
  const double nu_ = 1.0;
  const double dti_sub_ = 0.02;
  size_t nx_;
  size_t ny_;
  size_t nsub_;
  util::Duration dt_sub_;
  util::Duration dt_sub_half_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
