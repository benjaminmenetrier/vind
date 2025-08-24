/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <memory>
#include <ostream>
#include <string>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/memory/NonCopyable.h"

#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

namespace vind {
  class ModelAuxControl;
  class ModelAuxCtlVec;
  class ModelAuxIncrement;
  class Geometry;

// -----------------------------------------------------------------------------

class ModelAuxCovariance : public util::Printable,
                           private eckit::NonCopyable,
                           private util::ObjectCounter<ModelAuxCovariance> {
 public:
  static const std::string classname()
    {return "vind::ModelAuxCovariance";}

// Constructor/destructor
  ModelAuxCovariance(const eckit::Configuration & conf,
                     const Geometry &): conf_(conf)
    {}
  ~ModelAuxCovariance()
    {}

// Linear algebra operators
  void linearize(const ModelAuxControl &,
                 const Geometry &)
    {}
  void multiply(const ModelAuxIncrement &,
                ModelAuxIncrement &) const
    {}
  void inverseMultiply(const ModelAuxIncrement &,
                       ModelAuxIncrement &) const
    {}
  void multiplySqrt(const ModelAuxCtlVec &,
                    ModelAuxIncrement &) const
    {}
  void multiplySqrtTrans(const ModelAuxIncrement &,
                         ModelAuxCtlVec &) const
    {}
  void randomize(ModelAuxIncrement &) const
    {}

// Return configuration
  const eckit::Configuration & config() const
    {return conf_;}

 private:
  void print(std::ostream & os) const
    {}

  const eckit::LocalConfiguration conf_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
