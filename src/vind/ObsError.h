/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "oops/interface/ObsAuxControlBase.h"
#include "oops/interface/ObsErrorBase.h"

#include "vind/ObsVector.h"
#include "vind/TraitsFwd.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class ObsSpace;

// -----------------------------------------------------------------------------
/// ObsError class

class ObsError : public oops::ObsErrorBase<Traits> {
  using ObsAuxControlPtrMap_ =
    typename std::map<std::string, std::unique_ptr<oops::ObsAuxControlBase<Traits>>>;

 public:
  static const std::string classname()
    {return "vind::ObsError";}

/// Constructor, destructor
  ObsError(const ObsSpace &,
           const eckit::Configuration &);
  ~ObsError()
    {}

/// Linearize and reset for inner loop
  void linearize(const ObsVector &)
    {}
  void linearize(const ObsAuxControlPtrMap_ &,
                 const ObsVector &dy)
    {}

/// Setup VarQC weights
  void setupWeights(const ObsVector &);

/// Multiply a Departure by \f$R\f$
  ObsVector * multiply(const ObsVector &) const;

/// Multiply a Departure by \f$R^{-1}\f$
  ObsVector * inverseMultiply(const ObsVector &) const;

/// Multiply a Departure by \f$W^1/2\f$
  ObsVector * multiplyWghtSqrt(const ObsVector &) const;

/// Generate random perturbation
  void randomize(ObsVector &) const;

/// Get mean error for Jo table
  double getRMSE() const
    {return stddev_->rms();}

/// Save obs errors
  void save(const std::string &) const {}  // TODO(Benjamin)

/// Returns inverse of observation error variance
  void inverseVariance(ObsVector &dy) const
    {dy = *inverseVariance_;}

 private:
  void print(std::ostream &) const;

  mutable bool lvarqc_;
  mutable double cleft_;
  mutable double cright_;
  std::unique_ptr<ObsVector> stddev_;
  std::unique_ptr<ObsVector> inverseVariance_;
  std::unique_ptr<ObsVector> wghtsqrt_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
