/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <string>

#include "eckit/exception/Exceptions.h"

#include "vind/TraitsFwd.h"

#include "oops/interface/GenericMatrix.h"
#include "oops/util/ObjectCounter.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Increment;
  class State;

// -----------------------------------------------------------------------------
/// HybridWeight class

class HybridWeight : public oops::GenericMatrix<Traits>,
                     private util::ObjectCounter<HybridWeight> {
 public:
  static const std::string classname()
    {return "vind::HybridWeight";}

/// OOPS interface

// Constructor/destructor
  explicit HybridWeight(const eckit::Configuration &);
  ~HybridWeight()
    {}

// Multiply, inverse and inverse adjoint
  void multiply(State &) const
    {throw eckit::NotImplemented(Here());}
  void multiply(Increment &) const;
  void inverseMultiply(Increment &) const
    {throw eckit::NotImplemented(Here());}
  void transposeInverseMultiply(Increment &) const
    {throw eckit::NotImplemented(Here());}

 private:
  void print(std::ostream & os) const
    {os << std::endl << "Hybrid weight:" << wgt_;}

  double wgt_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
