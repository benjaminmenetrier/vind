/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/HybridWeight.h"

#include <cmath>
#include <vector>

#include "vind/Increment.h"

#include "oops/util/Logger.h"

namespace vind {

// -----------------------------------------------------------------------------

HybridWeight::HybridWeight(const eckit::Configuration & config)
  : wgt_(std::sqrt(config.getDouble("weight"))) {
  oops::Log::trace() << classname() << "::HybridWeight" << std::endl;
}


// -----------------------------------------------------------------------------

void HybridWeight::multiply(Increment & dx) const {
  oops::Log::trace() << classname() << "::multiply starting" << std::endl;

  dx *= wgt_;

  oops::Log::trace() << classname() << "::multiply done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
