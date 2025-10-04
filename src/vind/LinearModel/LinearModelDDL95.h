/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <ostream>
#include <string>

#include "oops/util/DateTime.h"
#include "oops/util/Duration.h"
#include "oops/util/ObjectCounter.h"

#include "vind/LinearModel/LinearModelBase.h"
#include "vind/Model/ModelDDL95.h"

namespace vind {
  class Geometry;
  class Increment;
  class ModelAuxControl;
  class ModelAuxIncrement;
  class State;

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

  // Set the linearization trajectory
  void setTrajectory(const State &,
                     State &,
                     const ModelAuxControl &) override;

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
    {return timeResolution_;}

 private:
  void print(std::ostream &) const override;
  void tendencyTL(const Increment &,
                  const State &,
                  Increment &) const;
  void tendencyAD(const Increment &,
                  const State &,
                  Increment &) const;

  const util::Duration timeResolution_;
  const double nu_ = 1.0;
  size_t nx_;
  size_t ny_;
  size_t ixMin_;
  size_t ixMax_;
  size_t iyMin_;
  size_t iyMax_;
  double dti_;
  util::Duration dt_half_;
  std::map<util::DateTime, State> traj_;
  std::unique_ptr<ModelDDL95> model_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
