/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <memory>
#include <string>

#ifdef READFA
#include "ectrans/transi.h"
#endif

#include "vind/FieldsIO/FieldsIOBase.h"

namespace vind {
  class Fields;

// -----------------------------------------------------------------------------
///  FieldsIOArome class

class FieldsIOArome : public FieldsIOBase {
 public:
  static const std::string classname()
    {return "vind::FieldsIOArome";}

  // Constructor/destructor
  explicit FieldsIOArome(const std::string & ioFormat)
    : FieldsIOBase(ioFormat) {}
  ~FieldsIOArome() = default;

  // Read
  void read(const oops::Variables &,
            const eckit::Configuration &,
            Fields &) const override;

  // Write
  void write(const eckit::Configuration &,
             const Fields &) const override;

 private:
#ifdef READFA
  std::unique_ptr<struct Trans_t> trans_;
#endif
};

// -----------------------------------------------------------------------------

}  // namespace vind

