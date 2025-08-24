/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "oops/util/Logger.h"

#include "vind/Increment.h"
#include "vind/LinearModel.h"
#include "vind/TraitsFwd.h"

namespace vind {

// -----------------------------------------------------------------------------

static oops::LinearModelMaker<Traits, LinearModel> makerLinearModelDefault_("default");

// -----------------------------------------------------------------------------

LinearModel::LinearModel(const Geometry &,
                         const eckit::Configuration & config)
  : timeResolution_(config.getString("tstep")) {
  oops::Log::trace() << classname() << "::LinearModel starting" << std::endl;

  oops::Log::info() << "Persistance linear model" << std::endl;

  oops::Log::trace() << classname() << "::LinearModel done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModel::stepTL(Increment & dx,
                         const ModelAuxIncrement &) const {
  oops::Log::trace() << classname() << "::stepTL starting" << std::endl;

  dx.updateTime(timeResolution_);

  oops::Log::trace() << classname() << "::stepTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModel::stepAD(Increment & dx,
                         ModelAuxIncrement &) const {
  oops::Log::trace() << classname() << "::stepAD starting" << std::endl;

  dx.updateTime(-timeResolution_);

  oops::Log::trace() << classname() << "::stepAD done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
