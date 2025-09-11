/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <string>
#include <vector>

#include "atlas/field.h"

#include "eckit/config/Configuration.h"

#include "oops/base/Variables.h"

#include "vind/FieldsIO/FieldsIOBase.h"

namespace vind {
  class Fields;

// -----------------------------------------------------------------------------
///  FieldsIOAQ class

class FieldsIOAQ : public FieldsIOBase {
 public:
  static const std::string classname()
    {return "vind::FieldsIOAQ";}

  // Constructor/destructor
  explicit FieldsIOAQ(const std::string & ioFormat)
    : FieldsIOBase(ioFormat) {}
  ~FieldsIOAQ() = default;

  // Read
  void read(const oops::Variables &,
            const eckit::Configuration &,
            Fields &) const override;

  // Write
  void write(const eckit::Configuration &,
             const Fields &) const override;
};

// -----------------------------------------------------------------------------

}  // namespace vind
