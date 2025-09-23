/*
 * (C) Copyright 2024-     UCAR.
 * (C) Copyright 2023-2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vader/vader.h"

#include "vind/Geometry.h"

namespace eckit {
  class Configuration;
}

namespace oops {
  class Variables;
}

namespace vind {
  class State;

// -----------------------------------------------------------------------------
/// VariableChange class

class VariableChange : public util::Printable,
                       private util::ObjectCounter<VariableChange> {
 public:
  static const std::string classname()
    {return "vind::VariableChange";}

  // Constructor/destructor
  VariableChange(const eckit::Configuration &,
                 const Geometry &);
  ~VariableChange()
    {}

  // Variable changes: direct and inverse
  void changeVar(State &,
                 const oops::Variables &) const;
  void changeVarInverse(State &,
                        const oops::Variables &) const;

 private:
  // Print
  void print(std::ostream & os) const override
    {os << "VariableChange";};

  // Geometry reference
  const Geometry & geom_;

  // VADER
  std::unique_ptr<vader::Vader> vader_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
