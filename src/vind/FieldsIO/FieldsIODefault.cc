/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "vind/FieldsIO/FieldsIODefault.h"

#include <vector>

#include "oops/util/FieldSetHelpers.h"
#include "oops/util/Logger.h"

#include "vind/Fields.h"

namespace vind {

// -----------------------------------------------------------------------------

static FieldsIOMaker<FieldsIODefault> makerDefault_("default");

// -----------------------------------------------------------------------------

void FieldsIODefault::read(const oops::Variables & vars,
                           const eckit::Configuration & conf,
                           Fields & fields) const {
  oops::Log::trace() << classname() << "::read starting" << std::endl;

  // Create variableSizes
  std::vector<size_t> variableSizes;
  for (const auto & var : vars) {
    variableSizes.push_back(var.getLevels());
  }

  // Update configuration
  eckit::LocalConfiguration updatedConf(conf);
  if (!updatedConf.has("latitude south to north")) {
    updatedConf.set("latitude south to north",
      fields.geometry()->io().getBool("latitude south to north", true));
  }

  // Read fieldset
  util::readFieldSet(fields.geometry()->getComm(),
                     fields.geometry()->functionSpace(),
                     variableSizes,
                     vars.variables(),
                     updatedConf,
                     fields.fieldSet());

  oops::Log::trace() << classname() << "::read done" << std::endl;
}

// -----------------------------------------------------------------------------

void FieldsIODefault::write(const eckit::Configuration & conf,
                            const Fields & fields) const {
  oops::Log::trace() << classname() << "::write starting" << std::endl;

  // Update configuration
  eckit::LocalConfiguration updatedConf(conf);
  if (!updatedConf.has("latitude south to north")) {
    updatedConf.set("latitude south to north",
      fields.geometry()->io().getBool("latitude south to north", true));
  }

  // Write fieldset
  util::writeFieldSet(fields.geometry()->getComm(), updatedConf, fields.fieldSet());

  oops::Log::trace() << classname() << "::write done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
