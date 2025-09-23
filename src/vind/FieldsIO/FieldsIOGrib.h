/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <string>

#include "vind/FieldsIO/FieldsIOBase.h"

namespace vind {
  class Fields;

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
  void read(const oops::Variables &,
            const eckit::Configuration &,
            Fields &) const override;
};

// -----------------------------------------------------------------------------

}  // namespace vind

