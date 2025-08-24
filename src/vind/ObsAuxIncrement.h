/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <iostream>
#include <string>

#include "eckit/mpi/Comm.h"

#include "oops/interface/ObsAuxIncrementBase.h"
#include "oops/util/Printable.h"

#include "vind/TraitsFwd.h"

namespace oops {
  template <typename MODEL>
  class ObsAuxControlBase;
}

namespace eckit {
  class Configuration;
}

namespace vind {
  class ObsAuxControl;

// -----------------------------------------------------------------------------

class ObsAuxIncrement : public oops::ObsAuxIncrementBase<Traits> {
 public:
  static const std::string classname()
    {return "vind::ObsAuxIncrement";}

/// Constructor, destructor
  ObsAuxIncrement(const ObsSpace &,
                  const eckit::Configuration &);
  ObsAuxIncrement(const ObsAuxIncrement &,
                  const bool copy = true);
  ObsAuxIncrement(const ObsAuxIncrement &,
                  const eckit::Configuration &);
  virtual ~ObsAuxIncrement()
    {}

/// Cloning
  virtual ObsAuxIncrement* clone(const bool copy = true) const
    {return new ObsAuxIncrement(*this, copy);}
  virtual ObsAuxIncrement* clone(const eckit::Configuration & conf) const
    {return new ObsAuxIncrement(*this, conf);}

/// Linear algebra operators
  void diff(const oops::ObsAuxControlBase<Traits> &,
            const oops::ObsAuxControlBase<Traits> &);
  void zero()
    {bias_ = 0.0;}
  ObsAuxIncrement & operator=(const oops::ObsAuxIncrementBase<Traits> &);
  ObsAuxIncrement & operator+=(const oops::ObsAuxIncrementBase<Traits> &);
  ObsAuxIncrement & operator-=(const oops::ObsAuxIncrementBase<Traits> &);
  ObsAuxIncrement & operator*=(const double &);
  void axpy(const double &,
            const oops::ObsAuxIncrementBase<Traits> &);
  double dot_product_with(const oops::ObsAuxIncrementBase<Traits> &) const;

/// I/O and diagnostics
  void read(const eckit::Configuration &)
    {}
  void write(const eckit::Configuration &) const
    {}
  double norm() const
    {return std::abs(bias_);}

/// Interfacing
  const int & getAccess() const
    {return dummyKey_;}
  int & getAccess()
    {return dummyKey_;}

/// Local
  ObsAuxIncrement & operator=(const ObsAuxIncrement &);
  double & value()
    {return bias_;}
  const double & value() const
    {return bias_;}

 private:
  void print(std::ostream &) const;

  const eckit::mpi::Comm & comm_;
  double bias_;
  bool active_;
  int dummyKey_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
