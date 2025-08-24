/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <map>
#include <memory>
#include <ostream>
#include <string>

#include "oops/interface/LinearObsOperBase.h"
#include "oops/util/ObjectCounter.h"

#include "vind/TraitsFwd.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class GeoVaLs;
  class ObsAuxControl;
  class ObsAuxIncrement;
  class ObsSpace;
  class ObsVector;
  class Variables;

// -----------------------------------------------------------------------------
/// LinearObsOperator class

class LinearObsOperator : public oops::LinearObsOperBase<Traits>,
                          private util::ObjectCounter<LinearObsOperator> {
  using ObsAuxControlPtrMap_ =
    typename std::map<std::string,
                      std::unique_ptr<oops::ObsAuxControlBase<Traits>> >;
  using ObsAuxIncrementPtrMap_ =
    typename std::map<std::string,
                      std::unique_ptr<oops::ObsAuxIncrementBase<Traits>> >;

 public:
  static const std::string classname()
    {return "vind::LinearObsOperator";}

/// OOPS interface

// Constructor/destructor
  LinearObsOperator(const ObsSpace &,
                    const eckit::Configuration &);
  ~LinearObsOperator()
    {}

// Trajectory
  void setTrajectory(const GeoVaLs &,
                     const ObsAuxControlPtrMap_ &) override
    {}

// Obs. equivalents
  void obsEquivTL(const GeoVaLs &,
                  ObsVector &,
                  const ObsAuxIncrementPtrMap_ &) const;
  void obsEquivAD(GeoVaLs &,
                  const ObsVector &,
                  ObsAuxIncrementPtrMap_ &) const;
  void obsBiasEquivTL(const GeoVaLs &,
                      ObsVector &,
                      const ObsAuxIncrementPtrMap_ &) const
    {}
  void obsBiasEquivAD(const GeoVaLs &,
                      const ObsVector &,
                      ObsAuxIncrementPtrMap_ &) const
    {}

// Variables accessor
  std::shared_ptr<const Variables> variables() const override
    {return inputs_;}

 private:
  void print(std::ostream &) const override;

  std::shared_ptr<const Variables> inputs_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
