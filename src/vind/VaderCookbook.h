/*
 * (C) Copyright 2023 UCAR
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

namespace vind {

// -------------------------------------------------------------------------------------------------

static std::map<std::string, std::vector<std::string>> vaderVindCustomCookbook() {
  return
    // pt: from t and pkz
    {{"air_potential_temperature",        {"AirPotentialTemperature_B"}},
    // P: from delp, from ps (and ak/bk)
    {"air_pressure_levels",          {"AirPressureAtInterface_B", "AirPressureAtInterface_A"}},
    // t: from p-pt and pt-base, from pt
    {"air_temperature",              {"AirTemperature_C", "AirTemperature_A"}},
    // p: from pe, from p-p and p-base
    {"air_pressure",                 {"AirPressure_A"}},
    // rh:
    {"relative_humidity",            {"RelativeHumidity_A"}},
    // sulfmf:
    {"mass_fraction_of_sulfate_in_air",  {"SulfateMassFraction_A"}},
    // mr: from spfh
    {"water_vapor_mixing_ratio_wrt_dry_air", {"WaterVaporMixingRatioWrtDryAir_C"}},
    // spfh: from mr
    {"water_vapor_mixing_ratio_wrt_moist_air",   {"WaterVaporMixingRatioWrtMoistAir_A"}},
    // total_water_mixing_ratio_wrt_dry_air
    {"total_water_mixing_ratio_wrt_dry_air", {"TotalWaterMixingRatioWrtDryAir_A"}},
    // total_water_mixing_ratio_wrt_moist_air_and_condensed_water
    {"total_water_mixing_ratio_wrt_moist_air_and_condensed_water",
               {"TotalWaterMixingRatioWrtWetAir_A"}},
    // ln(p) from pe
    {"ln_air_pressure_at_interface", {"LnAirPressureAtInterface_A"}},
    // qsat
    {"qsat",                         {"SaturationSpecificHumidity_A"}},
    // svp
    {"svp",                          {"SaturationVaporPressure_A"}},
    // dlsvpdT
    {"dlsvpdT",                      {"LogDerivativeSaturationVaporPressure_A"}},
    // p^kappa from pe and ln(p)
    {"air_pressure_to_kappa",        {"AirPressureToKappa_A"}},
    // delp: from p
    {"air_pressure_thickness",       {"AirPressureThickness_A"}},
    // pt: from t and ps
    {"potential_temperature",        {"AirPotentialTemperature_A"}},
    // ps: from delp
    {"air_pressure_at_surface",             {"SurfaceAirPressure_A"}},
    // tv: from t and q
    {"virtual_temperature",          {"AirVirtualTemperature_A"}}};
}

// -------------------------------------------------------------------------------------------------

}  // namespace vind
