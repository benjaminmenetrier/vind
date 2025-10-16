/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "eckit/memory/NonCopyable.h"

#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"
#include "oops/util/Serializable.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class ModelAuxIncrement;

// -----------------------------------------------------------------------------
/// ModelAuxControl class

class ModelAuxControl : public util::Printable,
                        public util::Serializable,
                        private eckit::NonCopyable,
                        private util::ObjectCounter<ModelAuxControl> {
 public:
  static const std::string classname()
    {return "vind::ModelAuxControl";}

  // Constructors/destructor
  ModelAuxControl(const Geometry &,
                  const eckit::Configuration &)
    {}
  ModelAuxControl(const Geometry &,
                  const ModelAuxControl &)
    {}
  ModelAuxControl(const ModelAuxControl &,
                  const bool)
    {}
  ~ModelAuxControl()
    {}

  // Basic operator
  ModelAuxControl & operator+=(const ModelAuxIncrement &) {return *this;}

  // I/O and diagnostics
  void read(const eckit::Configuration &)
    {}
  void write(const eckit::Configuration &) const
    {}
  double norm() const
    {return 0.0;}

  // Serialization
  size_t serialSize() const
    {return 0;}
  void serialize(std::vector<double> &) const
    {}
  void deserialize(const std::vector<double> &,
                   size_t &)
    {}

 private:
  void print(std::ostream & os) const {}
};

// -----------------------------------------------------------------------------

}  // namespace vind
