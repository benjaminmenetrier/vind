/*
 * (C) Copyright 2025 Meteorologisk Institutt
 * 
 */

#include "vind/LinearModel/LinearModelDDL95.h"

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Increment.h"

namespace vind {

// -----------------------------------------------------------------------------

static LinearModelMaker<LinearModelDDL95> makerDDL95_("DDL95");

// -----------------------------------------------------------------------------

LinearModelDDL95::LinearModelDDL95(const Geometry &,
                                   const eckit::Configuration & config)
  : timeResolution_(config.getString("time step")),
  stepTrajectory_(config.getString("trajectory step")),
  DDL95Factor_(config.getDouble("DDL95 factor", 1.0)) {
  oops::Log::trace() << classname() << "::LinearModelDDL95" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::stepTL(Increment & dx,
                              const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepTL starting" << std::endl;

  // Scale input field
  dx.fields() *= DDL95Factor_;

  // Update valid time
  dx.updateTime(timeResolution_);

  oops::Log::trace() << classname() << "::stepTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::stepAD(Increment & dx,
                              const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepAD starting" << std::endl;

  // Scale input field
  dx.fields() *= DDL95Factor_;

  // Update valid time
  dx.updateTime(-timeResolution_);

  oops::Log::trace() << classname() << "::stepAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "DDL95 linear model:" << std::endl;
  os << "- dt = " << timeResolution_ << std::endl;
  os << "- DDL95 factor = " << DDL95Factor_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
