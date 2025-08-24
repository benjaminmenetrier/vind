/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/ObsAuxCtlVec.h"

#include "oops/interface/ObsAuxCovarianceBase.h"

#include "oops/util/Logger.h"

namespace vind {

// -----------------------------------------------------------------------------

static oops::ObsAuxCtlVecMaker<Traits, ObsAuxCtlVec> makeObsAuxCtlVecDefault_("default");

// -----------------------------------------------------------------------------

}  // namespace vind
