/*
 * (C) Copyright 2025 Meteorologisk Institutt
 * 
 */

#include "vind/Model/ModelPersistence.h"

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/State.h"

namespace vind {

// -----------------------------------------------------------------------------

static ModelMaker<ModelPersistence> makerPersistence_("persistence");

// -----------------------------------------------------------------------------

ModelPersistence::ModelPersistence(const Geometry &,
                                   const eckit::Configuration & config)
  : timeResolution_(config.getString("tstep")), sigma_(config.getDouble("sigma", 0.0)) {
  oops::Log::trace() << classname() << "::ModelPersistence" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelPersistence::initialize(State & xx) const {
  oops::Log::trace() << classname() << "::initialize starting" << std::endl;
  oops::Log::trace() << classname() << "::initialize done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelPersistence::step(State & xx,
                            const ModelAuxControl & xxAux) const {
  oops::Log::trace() << classname() << "::step starting" << std::endl;

  // Get random seed from time
  int YYYYMMDD;
  int hhmmss;
  xx.validTime().toYYYYMMDDhhmmss(YYYYMMDD, hhmmss);
  const int seed = YYYYMMDD*hhmmss;

  // Generate random field
  Fields randomField(xx.fields(), false);
  randomField.random(seed);

  // Add scaled random field
  xx.fields().axpy(sigma_, randomField);

  // Update valid time
  xx.validTime() += timeResolution_;

  oops::Log::trace() << classname() << "::step done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelPersistence::finalize(State & xx) const {
  oops::Log::trace() << classname() << "::finalize starting" << std::endl;
  oops::Log::trace() << classname() << "::finalize done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelPersistence::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "Persistence model:" << std::endl;
  os << "- dt = " << timeResolution_ << std::endl;
  os << "- sigma = " << sigma_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
