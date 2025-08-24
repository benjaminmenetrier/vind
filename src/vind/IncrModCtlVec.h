/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Covariance;

// -----------------------------------------------------------------------------
/// IncrModCtlVec class

class IncrModCtlVec : public util::Printable,
                      private util::ObjectCounter<IncrModCtlVec> {
 public:
  static const std::string classname()
    {return "vind::IncrModCtlVec";}

/// OOPS interface

// Constructor, destructor
  IncrModCtlVec()
    {throw eckit::NotImplemented(Here());}
  explicit IncrModCtlVec(const Covariance &)
    {throw eckit::NotImplemented(Here());}
  IncrModCtlVec(const Covariance &,
                const IncrModCtlVec &)
    {throw eckit::NotImplemented(Here());}
  IncrModCtlVec(const IncrModCtlVec &,
                const bool = true)
    {throw eckit::NotImplemented(Here());}
  virtual ~IncrModCtlVec()
    {}

// Basic operators
  void zero()
    {throw eckit::NotImplemented(Here());}
  IncrModCtlVec & operator =(const IncrModCtlVec &)
    {throw eckit::NotImplemented(Here()); return *this;}
  IncrModCtlVec & operator+=(const IncrModCtlVec &)
    {throw eckit::NotImplemented(Here()); return *this;}
  IncrModCtlVec & operator-=(const IncrModCtlVec &)
    {throw eckit::NotImplemented(Here()); return *this;}
  IncrModCtlVec & operator*=(const double &)
    {throw eckit::NotImplemented(Here()); return *this;}
  void axpy(const double &,
            const IncrModCtlVec &,
            const bool check = true)
    {throw eckit::NotImplemented(Here());}
  double dot_product_with(const IncrModCtlVec &) const
    {throw eckit::NotImplemented(Here()); return 0.0;}
  void schur_product_with(const IncrModCtlVec &)
    {throw eckit::NotImplemented(Here());}
  void random()
    {throw eckit::NotImplemented(Here());}

// Section
  void getSection(std::vector<double> &,
                  const int &,
                  const int &) const
    {throw eckit::NotImplemented(Here());}
  void setSection(const std::vector<double> &,
                  const int &,
                  const int &)
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
    {}
};
// -----------------------------------------------------------------------------

}  // namespace vind
