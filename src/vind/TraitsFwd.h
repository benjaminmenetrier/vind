/*
 * (C) Copyright 2022 UCAR.
 * (C) Copyright 2023-2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <string>

namespace oops {
class AtlasInterpolator;
}  // namespace oops

namespace vind {

class Covariance;
class Geometry;
class GeometryIterator;
class GeoVaLs;
class HorizScaleDecomposition;
class Increment;
class IncrEnsCtlVec;
class IncrModCtlVec;
class Interpolator;
class LinearVariableChange;
class LocalizationMatrix;
class Locations;
class Model;
class ModelAuxControl;
class ModelAuxControlEstimator;
class ModelAuxCovariance;
class ModelAuxCtlVec;
class ModelAuxIncrement;
class ModelData;
class ObsSpace;
class ObsVector;
class State;
class VariableChange;

struct Traits {
  static std::string name()
    {return "vind";}
  static std::string nameCovar()
    {return "vindCovariance";}

  using Covariance = vind::Covariance;
  using Geometry = vind::Geometry;
  using GeometryIterator = vind::GeometryIterator;
  using GeoVaLs = vind::GeoVaLs;
  using HorizScaleDecomposition = vind::HorizScaleDecomposition;
  using Increment = vind::Increment;
  using IncrEnsCtlVec = vind::IncrEnsCtlVec;
  using IncrModCtlVec = vind::IncrModCtlVec;
  using Interpolator = vind::Interpolator;
  using LinearVariableChange = vind::LinearVariableChange;
//  using LocalInterpolator = oops::AtlasInterpolator;  // TODO(Benjamin)
  using LocalizationMatrix = vind::LocalizationMatrix;
  using Locations = vind::Locations;
  using Model = vind::Model;
  using ModelAuxControl = vind::ModelAuxControl;
  using ModelAuxControlEstimator = vind::ModelAuxControlEstimator;
  using ModelAuxCovariance = vind::ModelAuxCovariance;
  using ModelAuxCtlVec = vind::ModelAuxCtlVec;
  using ModelAuxIncrement = vind::ModelAuxIncrement;
  using ModelData = vind::ModelData;
  using ObsSpace = vind::ObsSpace;
  using ObsVector = vind::ObsVector;
  using State = vind::State;
  using VariableChange = vind::VariableChange;
};

}  // namespace vind
