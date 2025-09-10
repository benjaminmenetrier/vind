/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "atlas/field.h"

#include "eckit/config/Configuration.h"

#include "oops/base/Variables.h"

#include "vind/FieldsIO/FieldsIOBase.h"

namespace vind {
  class Fields;

// -----------------------------------------------------------------------------
///  FieldsIOBSC class

class FieldsIOBSC : public FieldsIOBase {
 public:
  static const std::string classname()
    {return "vind::FieldsIOBSC";}

  // Constructor/destructor
  explicit FieldsIOBSC(const std::string & ioFormat)
    : FieldsIOBase(ioFormat) {}
  ~FieldsIOBSC() = default;

  // Read
  void read(const oops::Variables &,
            const eckit::Configuration &,
            Fields &) const override;

  // Write
  void write(const eckit::Configuration &,
             const Fields &) const override;

 private:
  // Attribute storage API (NetCDF-independent)
  struct Attr {
    int type{0};  // Stores NetCDF nc_type as integer
    std::vector<std::string> tokens;
  };

  // Set attributes
  void setAttributes(eckit::LocalConfiguration &,
                     const std::map<std::string, Attr> &) const;

  // Get attributes
  const std::map<std::string, Attr> getAttributes(const eckit::LocalConfiguration &) const;

  // Write attributes
  void writeAttributesTyped(const int &,
                            const int &,
                            const eckit::LocalConfiguration &,
                            int &) const;
};

// -----------------------------------------------------------------------------

}  // namespace vind
