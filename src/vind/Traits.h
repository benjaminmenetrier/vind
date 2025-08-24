/*
 * (C) Copyright 2022 UCAR.
 * (C) Copyright 2023-2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include "oops/generic/AtlasInterpolator.h"
#include "vind/Covariance.h"
#include "vind/Geometry.h"
#include "vind/GeometryIterator.h"
#include "vind/GeoVaLs.h"
#include "vind/HorizScaleDecomposition.h"
#include "vind/Increment.h"
#include "vind/IncrEnsCtlVec.h"
#include "vind/IncrModCtlVec.h"
#include "vind/Interpolator.h"
#include "vind/LinearVariableChange.h"
#include "vind/LocalizationMatrix.h"
#include "vind/Locations.h"
#include "vind/Model.h"
#include "vind/ModelAuxControl.h"
#include "vind/ModelAuxControlEstimator.h"
#include "vind/ModelAuxCovariance.h"
#include "vind/ModelAuxCtlVec.h"
#include "vind/ModelAuxIncrement.h"
#include "vind/ModelData.h"
#include "vind/ObsSpace.h"
#include "vind/ObsVector.h"
#include "vind/State.h"
#include "vind/TraitsFwd.h"
#include "vind/VariableChange.h"
#ifdef ECSABER
#include "vind/Variables.h"
#endif
