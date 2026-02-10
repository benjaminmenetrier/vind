/*
 * (C) Copyright 2022 UCAR.
 * (C) Copyright 2023-2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "oops/util/parameters/OptionalParameter.h"
#include "oops/util/parameters/Parameter.h"
#include "oops/util/parameters/Parameters.h"
#include "oops/util/parameters/RequiredParameter.h"

namespace vind {

// -----------------------------------------------------------------------------
/// Orography parameters

class OrographyParameters : public oops::Parameters {
  OOPS_CONCRETE_PARAMETERS(OrographyParameters, Parameters)

 public:
  // Top longitude [degrees]
  oops::RequiredParameter<double> topLon{"top longitude", this};

  // Top latitude [degrees]
  oops::RequiredParameter<double> topLat{"top latitude", this};

  // Zonal length [m]
  oops::RequiredParameter<double> zonalLength{"zonal length", this};

  // Meridional length [m]
  oops::RequiredParameter<double> meridionalLength{"meridional length", this};

  // Height (% of the bottom layer thickness, or absolute value if one level only)
  oops::RequiredParameter<double> height{"height", this};
};

// -----------------------------------------------------------------------------
/// Group parameters

class GroupParameters : public oops::Parameters {
  OOPS_CONCRETE_PARAMETERS(GroupParameters, Parameters)

 public:
  // Variables
  oops::RequiredParameter<std::vector<std::string>> variables{"variables", this};

  // Number of levels
  oops::Parameter<size_t> levels{"levels", 1, this};

  // Corresponding level for 2D variables (first or last)
  oops::Parameter<std::string> lev2d{"lev2d", "first", this};

  // Orography
  oops::OptionalParameter<OrographyParameters> orography{"orography", this};

  // Vertical coordinate configuration
  oops::OptionalParameter<eckit::LocalConfiguration> vertCoordConf{
    "vertical coordinate", this};

  // Mask type
  oops::Parameter<std::string> maskType{"mask type", "none", this};

  // Mask path
  oops::OptionalParameter<std::string> maskPath{"mask path", this};
};

// -----------------------------------------------------------------------------
/// Alias elemental paramaters

class AliasParameters : public oops::Parameters {
  OOPS_CONCRETE_PARAMETERS(AliasParameters, oops::Parameters)

 public:
  // In code
  oops::RequiredParameter<std::string> inCode{"in code", this};
  // In model file
  oops::RequiredParameter<std::string> inFile{"in file", this};
};

// -----------------------------------------------------------------------------
/// Transform elemental paramaters

class TransformParameters : public oops::Parameters {
  OOPS_CONCRETE_PARAMETERS(TransformParameters, oops::Parameters)

 public:
  // Variable name
  oops::RequiredParameter<std::string> variable{"variable", this};
  // Scaling factor (e.g. for units conversion)
  oops::OptionalParameter<double> scalingFactor{"scaling factor", this};
  // Toggle log10 transformation
  oops::OptionalParameter<bool> logTransf{"log transform", this};
  // Additive constant (prior to log10 transformation)
  oops::OptionalParameter<double> addConst{"additive constant", this};
};

// -----------------------------------------------------------------------------
/// Interpolation paramaters

class InterpolationParameters : public oops::Parameters {
  OOPS_CONCRETE_PARAMETERS(InterpolationParameters, oops::Parameters)

 public:
  // Interpolation type
  oops::RequiredParameter<std::string> interpType{"interpolation type", this};
};

// -----------------------------------------------------------------------------
/// Geometry parameters

class GeometryParameters : public oops::Parameters {
  OOPS_CONCRETE_PARAMETERS(GeometryParameters, Parameters)

 public:
  // Function space
  oops::RequiredParameter<std::string> functionSpace{"function space", this};

  // Grid
  oops::RequiredParameter<eckit::LocalConfiguration> grid{"grid", this};

  // Partitioner
  oops::Parameter<std::string> partitioner{"partitioner", "equal_regions", this};

  // Variables groups
  oops::RequiredParameter<std::vector<GroupParameters>> groups{"groups", this};

  // Halo size
  oops::Parameter<size_t> halo{"halo", 0, this};

  // No point on last task
  oops::Parameter<bool> noPointOnLastTask{"no point on last task", false, this};

  // Levels top-down
  oops::Parameter<bool> levelsAreTopDown{"levels are top down", true, this};

  // Levels counter origin
  oops::Parameter<size_t> levelsCountFrom{"levels count from", 1, this};

  // Model data
  oops::Parameter<eckit::LocalConfiguration> modelData{"model data", eckit::LocalConfiguration(),
    this};

  // Variable alias (different name in file and in code)
  oops::Parameter<std::vector<AliasParameters>> alias{"alias", {}, this};

  // Variable transform
  oops::Parameter<std::vector<TransformParameters>> transform{"transform", {}, this};

  // Check longitudes/latitudes from file
  oops::OptionalParameter<eckit::LocalConfiguration> checkLonLat{"check lon/lat from file", this};

  // IO parameters
  oops::Parameter<eckit::LocalConfiguration> io{"io", eckit::LocalConfiguration(), this};

  // Interpolation parameters
  oops::OptionalParameter<InterpolationParameters> interpolation{"interpolation", this};

  // Geometry iterator common vertical coordinate
  oops::Parameter<std::string> commonVertCoord{"common vertical coordinate", "vert_coord_0", this};

  // Geometry iterator dimension
  oops::Parameter<size_t> iteratorDim{"iterator dimension", 2, this};

  // Helpers

  // Return variable name in model file
  std::string fileAlias(const std::string & inCode) const {
    for (const auto & item : alias.value()) {
      if (item.inCode.value() == inCode) {
        return item.inFile.value();
      }
    }
    // Default value: input
    return inCode;
  }

  // Return variable name in code
  std::string codeAlias(const std::string & inFile) const {
    for (const auto & item : alias.value()) {
      if (item.inFile.value() == inFile) {
        return item.inCode.value();
      }
    }
    // Default value: input
    return inFile;
  }

  // Return scaling factor
  double scalingFactor(const std::string & varName) const {
    for (const auto & item : transform.value()) {
      if (item.variable.value() == varName) {
        if (item.scalingFactor.value()) {
          return *item.scalingFactor.value();
        }
      }
    }
    // Default value: 1.0
    return 1.0;
  }

  // Return toggle for log10 transformation
  bool logTransf(const std::string & varName) const {
    for (const auto & item : transform.value()) {
      if (item.variable.value() == varName) {
        if (item.logTransf.value()) {
          return *item.logTransf.value();
        }
      }
    }
    // Default value: false
    return false;
  }

  // Return scaling factor
  double addConst(const std::string & varName) const {
    for (const auto & item : transform.value()) {
      if (item.variable.value() == varName) {
        if (item.addConst.value()) {
          return *item.addConst.value();
        }
      }
    }
    // Default value: 0.0
    return 0.0;
  }
};

// -----------------------------------------------------------------------------

}  // namespace vind
