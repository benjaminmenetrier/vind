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
  class Increment;

// -----------------------------------------------------------------------------
/// HorizScaleDecomposition class

class HorizScaleDecomposition: public util::Printable,
                               private eckit::NonCopyable,
                               private util::ObjectCounter<HorizScaleDecomposition> {
 public:
  static const std::string classname()
    {return "vind::HorizScaleDecomposition";}

/// OOPS interface
  explicit HorizScaleDecomposition(const eckit::Configuration &)
    {throw eckit::NotImplemented(Here());}

  void collect(const Increment &,
               const int &) const
    {throw eckit::NotImplemented(Here());}
  void decompose(const Increment &,
                 const int &) const
    {throw eckit::NotImplemented(Here());}

 private:
  void print(std::ostream & os) const
    {throw eckit::NotImplemented(Here());}
};
// -----------------------------------------------------------------------------

}  // namespace vind
