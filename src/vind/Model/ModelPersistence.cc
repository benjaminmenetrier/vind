/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/Model/ModelPersistence.h"

#include "oops/util/Logger.h"

namespace vind {

// -----------------------------------------------------------------------------

static oops::interface::ModelMaker<Traits, ModelPersistence> makerPersistence_("persistence");

// -----------------------------------------------------------------------------

ModelPersistence::ModelPersistence(const Geometry &,
                                   const eckit::Configuration & config)
  : timeResolution_(config.getString("tstep")) {
  oops::Log::trace() << classname() << "::Model" << std::endl;
}

// -----------------------------------------------------------------------------
ModelPersistence::ModelPersistence(const ModelPersistence & other)
  : timeResolution_(other.timeResolution_) {
  oops::Log::trace() << classname() << "::Model" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelPersistence::step(State & xx,
                            const ModelAuxControl & xxAux) const {
  oops::Log::trace() << classname() << "::step starting" << std::endl;

  xx.validTime() += timeResolution_;

  oops::Log::trace() << classname() << "::step done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
