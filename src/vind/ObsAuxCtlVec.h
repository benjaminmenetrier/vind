/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "oops/interface/ObsAuxCtlVecBase.h"
#include "oops/util/Printable.h"

#include "vind/TraitsFwd.h"

namespace oops {
  template <typename MODEL>
  class ObsAuxCovarianceBase;
}

namespace eckit {
  class Configuration;
}

namespace vind {

// -----------------------------------------------------------------------------
/// ObsAuxCtlVec class

class ObsAuxCtlVec : public oops::ObsAuxCtlVecBase<Traits> {
 public:
  static const std::string classname()
    {return "vind::ObsAuxCtlVec";}

/// Constructor, destructor
  explicit ObsAuxCtlVec(const oops::ObsAuxCovarianceBase<Traits> &)
    {}
  ObsAuxCtlVec(const oops::ObsAuxCovarianceBase<Traits> &,
               const ObsAuxCtlVec &)
    {}
  ObsAuxCtlVec(const ObsAuxCtlVec &,
               const bool copy = true)
    {}
  virtual ~ObsAuxCtlVec()
    {}

/// Cloning
  virtual ObsAuxCtlVec* clone(const oops::ObsAuxCovarianceBase<Traits> & cov) const
    {return new ObsAuxCtlVec(cov, *this);}
  virtual ObsAuxCtlVec* clone(const bool copy = true) const
    {return new ObsAuxCtlVec(*this, copy);}

/// Linear algebra operators
  void zero()
    {}
  ObsAuxCtlVec & operator=(const oops::ObsAuxCtlVecBase<Traits> &)
    {return *this;}
  ObsAuxCtlVec & operator+=(const oops::ObsAuxCtlVecBase<Traits> &)
    {return *this;}
  ObsAuxCtlVec & operator-=(const oops::ObsAuxCtlVecBase<Traits> &)
    {return *this;}
  ObsAuxCtlVec & operator*=(const double &)
    {return *this;}
  void axpy(const double &,
            const oops::ObsAuxCtlVecBase<Traits> &)
    {}
  double dot_product_with(const oops::ObsAuxCtlVecBase<Traits> &) const
    {return 0.0;}

/// I/O and diagnostics
  void read(const eckit::Configuration &)
    {}
  void write(const eckit::Configuration &) const
    {}
  double norm() const
    {return 0.0;}

/// Interfacing
  const int & getAccess() const
    {return dummyKey_;}
  int & getAccess()
    {return dummyKey_;}

 private:
  void print(std::ostream &) const
    {}

  int dummyKey_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
