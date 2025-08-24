/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/Model.h"

#include "oops/util/Logger.h"

#include "vind/State.h"

namespace vind {

// -----------------------------------------------------------------------------

Model::Model(const Geometry &,
             const eckit::Configuration & config)
  : timeResolution_(config.getString("tstep")) {
  oops::Log::trace() << classname() << "::Model" << std::endl;
}

// -----------------------------------------------------------------------------
Model::Model(const Model & other)
  : timeResolution_(other.timeResolution_) {
  oops::Log::trace() << classname() << "::Model" << std::endl;
}

// -----------------------------------------------------------------------------

void Model::step(State & xx,
                 const ModelAuxControl &) const {
  oops::Log::trace() << classname() << "::step starting" << std::endl;

  xx.validTime() += timeResolution_;

  oops::Log::trace() << classname() << "::step done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
