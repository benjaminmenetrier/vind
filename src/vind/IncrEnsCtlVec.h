/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <memory>
#include <ostream>
#include <string>

#include "eckit/exception/Exceptions.h"

#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class LocalizationMatrix;

// -----------------------------------------------------------------------------
/// IncrEnsCtlVec class

class IncrEnsCtlVec : public util::Printable,
                      private util::ObjectCounter<IncrEnsCtlVec> {
 public:
  static const std::string classname()
    {return "vind::IncrEnsCtlVec";}

/// OOPS interface

// Constructor, destructor
  IncrEnsCtlVec()
    {throw eckit::NotImplemented(Here());}
  IncrEnsCtlVec(const Geometry &,
                const LocalizationMatrix &)
    {throw eckit::NotImplemented(Here());}
  IncrEnsCtlVec(const Geometry &,
                const LocalizationMatrix &, const IncrEnsCtlVec &)
    {throw eckit::NotImplemented(Here());}
  IncrEnsCtlVec(const IncrEnsCtlVec &,
                const bool)
    {throw eckit::NotImplemented(Here());}
  virtual ~IncrEnsCtlVec()
    {}

// Basic operators
  void zero()
    {throw eckit::NotImplemented(Here());}
  IncrEnsCtlVec & operator =(const IncrEnsCtlVec &)
    {throw eckit::NotImplemented(Here()); return *this;}
  IncrEnsCtlVec & operator+=(const IncrEnsCtlVec &)
    {throw eckit::NotImplemented(Here()); return *this;}
  IncrEnsCtlVec & operator-=(const IncrEnsCtlVec &)
    {throw eckit::NotImplemented(Here()); return *this;}
  IncrEnsCtlVec & operator*=(const double &)
    {throw eckit::NotImplemented(Here()); return *this;}
  void axpy(const double &,
            const IncrEnsCtlVec &,
            const bool check = true)
    {throw eckit::NotImplemented(Here());}
  double local_dot_product_with(const IncrEnsCtlVec &) const
    {throw eckit::NotImplemented(Here()); return 0.0;}
  void schur_product_with(const IncrEnsCtlVec &)
    {throw eckit::NotImplemented(Here());}
  void random()
    {throw eckit::NotImplemented(Here());}

// I/O and diagnostics
  void read(const eckit::Configuration &)
    {throw eckit::NotImplemented(Here());}
  void write(const eckit::Configuration &) const
    {throw eckit::NotImplemented(Here());}
  double norm() const
    {throw eckit::NotImplemented(Here()); return 0.0;}

 private:
  void print(std::ostream &) const
    {throw eckit::NotImplemented(Here());}
};
// -----------------------------------------------------------------------------

}  // namespace vind
