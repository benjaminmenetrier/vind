/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include "oops/interface/GenericMatrix.h"

#include "vind/HybridWeight.h"
#include "vind/TraitsFwd.h"

namespace vind {

// -----------------------------------------------------------------------------

void instantiateQuenchMatrices() {
  static oops::GenericMatrixMaker<vind::Traits, HybridWeight> makerEnsWgt_("hybrid_weight");
}

// -----------------------------------------------------------------------------

}  // namespace vind
