/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <memory>
#include <ostream>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/NonCopyable.h"

#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class Increment;
  class IncrEnsCtlVec;
  class Variables;

// -----------------------------------------------------------------------------
/// LocalizationMatrix class

class LocalizationMatrix: public util::Printable,
                          private eckit::NonCopyable,
                          private util::ObjectCounter<LocalizationMatrix> {
 public:
  static const std::string classname()
    {return "vind::LocalizationMatrix";}

/// OOPS interface
  LocalizationMatrix(const Geometry &,
                     const Variables &,
                     const eckit::Configuration &)
    {}
  ~LocalizationMatrix()
    {}

// Multiply
  void multiply(Increment &) const
    {}

// Square-root multiply and adjoint
  void multiplySqrt(const IncrEnsCtlVec &,
                    Increment &) const
    {throw eckit::NotImplemented(Here());}
  void multiplySqrtTrans(const Increment &,
                         IncrEnsCtlVec &) const
    {throw eckit::NotImplemented(Here());}

 private:
  void print(std::ostream &) const
    {throw eckit::NotImplemented(Here());}
};
// -----------------------------------------------------------------------------

}  // namespace vind
