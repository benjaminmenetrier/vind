/*
 * (C) Copyright 2024-     UCAR.
 * (C) Copyright 2023-2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "oops/util/parameters/OptionalParameter.h"
#include "oops/util/parameters/Parameters.h"

namespace vind {

// -------------------------------------------------------------------------------------------------
/// VariableChange parameters class

class VariableChangeParameters : public oops::Parameters {
  OOPS_CONCRETE_PARAMETERS(VariableChangeParameters, Parameters)
 public:
  oops::Parameter<std::map<std::string, std::vector<std::string>>> vaderCustomCookbook{
    "vader custom cookbook", vaderVindCustomCookbook(), this};
  oops::OptionalParameter<vader::VaderParameters> vaderParam{"vader", this};
};

// -------------------------------------------------------------------------------------------------

}  // namespace vind
