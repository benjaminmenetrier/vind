/*
 * (C) Copyright 2023 UCAR
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "vind/Constants.h"

#include <cmath>
#include <unordered_map>

#include "eckit/exception/Exceptions.h"

namespace vind {

// -------------------------------------------------------------------------------------------------

// Define the constants where they are used to compute other constants
static const double grav           = 9.80665;
static const double airmw          = 28.965;
static const double h2omw          = 18.015;
static const double sulfmw         = 96.06;
static const double runiv          = 8314.47;
static const double rdry           = runiv/airmw;
static const double cpdry          = 3.5*rdry;
static const double rvap           = runiv/h2omw;
static const double kappa          = rdry/cpdry;
static const double epsilon        = h2omw/airmw;
static const double zvir           = rvap/rdry - 1.;
static const double lapse_rate     = -0.0065;
static const double lapse_exponent = -(grav*0.0289644)/(runiv/1000*lapse_rate);

// Put the constants into a map
static const std::unordered_map<std::string, double> constants = {
  {"rad2deg", 57.2957779186820},
  {"pi", M_PI},
  {"tice", 273.16},
  {"constoz", 603447.6},
  {"ps", 101300.0},
  {"grav", grav},
  {"molecular_weight_of_air", airmw},
  {"molecular_weight_of_h2o", h2omw},
  {"molecular_weight_of_so4", sulfmw},
  {"runiv", runiv},
  {"rdry", rdry},
  {"cpdry", cpdry},
  {"rvap", rvap},
  {"kappa", kappa},
  {"epsilon", epsilon},
  {"zvir", zvir},
  {"lapse_rate", lapse_rate},
  {"lapse_exponent", lapse_exponent}
};

// Get constants
double getConstant(const std::string constName) {
  auto it = constants.find(constName);
  ASSERT_MSG(it != constants.end(), "Constants: Constant name "+constName+" is not found.");
  return it->second;
}

// Get constants names
std::vector<std::string> getAllConstantsNames() {
std::vector<std::string> allNames;
for (auto const& imap : constants) {
  allNames.push_back(imap.first);
}
return allNames;
}

// -------------------------------------------------------------------------------------------------

}  // namespace vind
