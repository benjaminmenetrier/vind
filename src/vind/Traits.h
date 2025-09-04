/*
 * (C) Copyright 2022 UCAR.
 * (C) Copyright 2023-2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <string>

#include "oops/generic/AtlasInterpolator.h"
#include "vind/Covariance.h"
#include "vind/Geometry.h"
#include "vind/GeometryIterator.h"
#include "vind/Increment.h"
#include "vind/LinearModel.h"
#include "vind/LinearVariableChange.h"
#include "vind/Model.h"
#include "vind/ModelAuxControl.h"
#include "vind/ModelAuxCovariance.h"
#include "vind/ModelAuxIncrement.h"
#include "vind/ModelData.h"
#include "vind/State.h"
#include "vind/VariableChange.h"

namespace oops {
class AtlasInterpolator;
}  // namespace oops

namespace vind {

struct Traits {
  static std::string name()
    {return "vind";}
  static std::string nameCovar()
    {return "vindCovariance";}

  typedef vind::Covariance           Covariance;
  typedef vind::Geometry             Geometry;
  typedef vind::GeometryIterator     GeometryIterator;
  typedef vind::Increment            Increment;
  typedef vind::LinearModel          LinearModel;
  typedef vind::LinearVariableChange LinearVariableChange;
  typedef vind::Model                Model;
  typedef vind::ModelAuxControl      ModelAuxControl;
  typedef vind::ModelAuxCovariance   ModelAuxCovariance;
  typedef vind::ModelAuxIncrement    ModelAuxIncrement;
  typedef vind::ModelData            ModelData;
//  typedef vind::NormGradient         NormGradient;
  typedef vind::State                State;
  typedef vind::VariableChange       VariableChange;
};

}  // namespace vind
