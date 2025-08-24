/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "oops/util/Printable.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class ModelAuxControl;
  class ModelAuxCovariance;

// -----------------------------------------------------------------------------

class ModelAuxIncrement : public util::Printable {
 public:
  static const std::string classname()
    {return "vind::ModelAuxIncrement";}

// Constructors/destructor
  ModelAuxIncrement(const Geometry &,
                    const eckit::Configuration &)
    {}
  ModelAuxIncrement(const ModelAuxIncrement &,
                    const bool = true)
    {}
  ModelAuxIncrement(const ModelAuxIncrement &,
                    const eckit::Configuration &)
    {}
  ~ModelAuxIncrement()
    {}

// Linear algebra operators
  void diff(const ModelAuxControl &,
            const ModelAuxControl &)
    {}
  void zero()
    {}
  ModelAuxIncrement & operator=(const ModelAuxIncrement &)
    {return *this;}
  ModelAuxIncrement & operator+=(const ModelAuxIncrement &)
    {return *this;}
  ModelAuxIncrement & operator-=(const ModelAuxIncrement &)
    {return *this;}
  ModelAuxIncrement & operator*=(const double &)
    {return *this;}
  void axpy(const double,
            const ModelAuxIncrement &)
    {}
  double dot_product_with(const ModelAuxIncrement &) const
    {return 0.0;}

/// Serialize-Deserialize
  size_t serialSize() {return 0;}
  void serialize(std::vector<double> & vect) const {}
  void deserialize(const std::vector<double> &,
                   size_t & index) {}

// I/O and diagnostics
  void read(const eckit::Configuration &)
    {}
  void write(const eckit::Configuration &) const
    {}
  double norm() const
    {return 0.0;}

 private:
  void print(std::ostream & os) const
    {}
};

// -----------------------------------------------------------------------------

}  // namespace vind
