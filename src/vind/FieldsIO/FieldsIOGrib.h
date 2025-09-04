/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <string>

#include "atlas/field.h"

#include "eckit/config/Configuration.h"

#include "oops/base/Variables.h"

#include "vind/FieldsIO/FieldsIOBase.h"

namespace vind {
  class Geometry;

// -----------------------------------------------------------------------------
///  FieldsIOGrib class

class FieldsIOGrib : public FieldsIOBase {
 public:
  static const std::string classname()
    {return "vind::FieldsIOGrib";}

  // Constructor/destructor
  explicit FieldsIOGrib(const std::string & ioFormat)
    : FieldsIOBase(ioFormat) {}
  ~FieldsIOGrib() = default;

  // Read
  void read(const Geometry &,
            const oops::Variables &,
            const eckit::Configuration &,
            atlas::FieldSet &) const override;
};

// -----------------------------------------------------------------------------

}  // namespace vind

