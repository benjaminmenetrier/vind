/*
 * (C) Copyright 2025 Meteorologisk Institutt
 * 
 */

#include "vind/LinearModel/LinearModelPersistence.h"

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Increment.h"

namespace vind {

// -----------------------------------------------------------------------------

static LinearModelMaker<LinearModelPersistence> makerPersistence_("persistence");

// -----------------------------------------------------------------------------

LinearModelPersistence::LinearModelPersistence(const Geometry &,
                                               const eckit::Configuration & config)
  : timeResolution_(config.getString("time step")),
  stepTrajectory_(config.getString("trajectory step")),
  persistenceFactor_(config.getDouble("persistence factor", 1.0)) {
  oops::Log::trace() << classname() << "::LinearModelPersistence" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPersistence::stepTL(Increment & dx,
                                    const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepTL starting" << std::endl;

  // Scale input field
  dx.fields() *= persistenceFactor_;

  // Update valid time
  dx.updateTime(timeResolution_);

  oops::Log::trace() << classname() << "::stepTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPersistence::stepAD(Increment & dx,
                                    const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepAD starting" << std::endl;

  // Scale input field
  dx.fields() *= persistenceFactor_;

  // Update valid time
  dx.updateTime(-timeResolution_);

  oops::Log::trace() << classname() << "::stepAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelPersistence::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "Persistence linear model:" << std::endl;
  os << "- dt = " << timeResolution_ << std::endl;
  os << "- persistence factor = " << persistenceFactor_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
