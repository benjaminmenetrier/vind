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

#include "eckit/config/Configuration.h"

#include "oops/base/Variables.h"
#include "oops/util/Printable.h"

#include "vind/Geometry.h"
#include "vind/State.h"

#include "vader/vader.h"

namespace vind {

// -----------------------------------------------------------------------------

class VariableChange : public util::Printable {
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
