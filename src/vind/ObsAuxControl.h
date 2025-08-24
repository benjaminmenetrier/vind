/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <cmath>
#include <iostream>
#include <string>

#include "eckit/memory/NonCopyable.h"

#include "oops/interface/ObsAuxControlBase.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vind/TraitsFwd.h"

namespace oops {
  template <typename MODEL>
  class ObsAuxIncrementBase;
}

namespace eckit {
  class Configuration;
}

namespace vind {

// -----------------------------------------------------------------------------
/// ObsAuxControl class

class ObsAuxControl : public oops::ObsAuxControlBase<Traits>,
                private eckit::NonCopyable,
                private util::ObjectCounter<ObsAuxControl> {
 public:
  static const std::string classname()
    {return "vind::ObsAuxControl";}

/// Constructor, destructor
  ObsAuxControl(const ObsSpace &,
                const eckit::Configuration &);
  ObsAuxControl(const ObsAuxControl &,
                const bool);
  virtual ~ObsAuxControl()
    {}

/// Cloning
  virtual ObsAuxControl* clone(const bool copy) const
    {return new ObsAuxControl(*this, copy);}

/// Operators
  oops::ObsAuxControlBase<Traits> & operator+=(const oops::ObsAuxIncrementBase<Traits> &);

/// Reset
  void reset(const ObsSpace &,
             const std::string &,
             const eckit::Configuration &)
    {}

/// I/O and diagnostics
  void read(const eckit::Configuration &)
    {}
  void write(const eckit::Configuration &) const
    {}
  double norm() const
    {return std::abs(bias_);}

/// Interfacing
  const int & getAccess() const
    {return key_;}
  int & getAccess()
    {return key_;}
  const int & getAccessToObsSpace() const
    {return obsSpaceKey_;}

/// Local
  const double & value() const
    {return bias_;}
  double & value()
    {return bias_;}

 private:
  void print(std::ostream &) const;

  double bias_;
  int obsSpaceKey_;
  int key_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
