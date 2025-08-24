/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <ostream>
#include <string>

#include "eckit/config/LocalConfiguration.h"

#include "oops/interface/ObsAuxCovarianceBase.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vind/TraitsFwd.h"

namespace oops {
  template <typename MODEL>
  class ObsAuxControlBase;

  template <typename MODEL>
  class ObsAuxCtlVecBase;

  template <typename MODEL>
  class ObsAuxIncrementBase;
}

namespace eckit {
  class Configuration;
}

namespace vind {
  class ObsAuxControl;
  class ObsAuxCtlVec;
  class ObsAuxIncrement;

// -----------------------------------------------------------------------------

class ObsAuxCovariance : public oops::ObsAuxCovarianceBase<Traits>,
                          private util::ObjectCounter<ObsAuxCovariance> {
 public:
  static const std::string classname()
    {return "vind::ObsAuxCovariance";}

/// Constructor, destructor
  ObsAuxCovariance(const ObsSpace &,
                   const eckit::Configuration &);
  virtual ~ObsAuxCovariance()
    {}

/// Update
  void update(const oops::ObsAuxControlBase<Traits> &,
              const eckit::Configuration &)
    {}

/// Linear algebra operators
  void linearize(const oops::ObsAuxControlBase<Traits> &)
    {}
  void multiply(const oops::ObsAuxIncrementBase<Traits> &,
                oops::ObsAuxIncrementBase<Traits> &) const;
  void inverseMultiply(const oops::ObsAuxIncrementBase<Traits> &,
                       oops::ObsAuxIncrementBase<Traits> &) const;
  void multiplySqrt(const oops::ObsAuxCtlVecBase<Traits> &,
                    oops::ObsAuxIncrementBase<Traits> &) const
    {}
  void multiplySqrtTrans(const oops::ObsAuxIncrementBase<Traits> &,
                         oops::ObsAuxCtlVecBase<Traits> &) const
    {}
  void randomize(oops::ObsAuxIncrementBase<Traits> &) const;

/// Interfacing
  const int & getAccess() const
    {return key_;}
  int & getAccess()
    {return key_;}
  const eckit::Configuration & config() const
    {return conf_;}

 private:
  void print(std::ostream &) const;

  const eckit::LocalConfiguration conf_;
  double variance_;
  bool active_;
  int key_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
