/*
 * (C) Copyright 2023 UCAR
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <string>
#include <vector>

namespace vind {

// -------------------------------------------------------------------------------------------------

// Get constants
double getConstant(const std::string constName);

// Get constants names
std::vector<std::string> getAllConstantsNames();

// -------------------------------------------------------------------------------------------------

}  // namespace vind
