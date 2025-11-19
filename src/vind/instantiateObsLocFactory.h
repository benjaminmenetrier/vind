/*
 * (C) Copyright 2025 Meteorologisk Institutt
 * 
 */

#pragma once

#include "ufo/instantiateObsLocFactory.h"
#include "ufo/obslocalization/ObsLocalizationBase.h"

#include "vind/GeometryIterator.h"

namespace vind {

void instantiateObsLocFactory() {
  ufo::instantiateObsLocFactory<GeometryIterator>();
}

}  // namespace vind
