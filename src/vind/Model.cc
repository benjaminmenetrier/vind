/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/Model.h"

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/State.h"

namespace vind {

// -----------------------------------------------------------------------------

Model::Model(const Geometry & geom,
             const eckit::Configuration & config)
  : model_() {
  oops::Log::trace() << classname() << "::Model starting" << std::endl;

  model_.reset(ModelFactory::create(geom, config));

  oops::Log::trace() << classname() << "::Model done" << std::endl;
}

// -----------------------------------------------------------------------------

void Model::initialize(State & xx) const {
  oops::Log::trace() << classname() << "::initialize starting" << std::endl;

  model_->initialize(xx);

  oops::Log::trace() << classname() << "::initialize done" << std::endl;
}

// -----------------------------------------------------------------------------

void Model::step(State & xx,
                 const ModelAuxControl & xxAux) const {
  oops::Log::trace() << classname() << "::step starting" << std::endl;

  model_->step(xx, xxAux);

  oops::Log::trace() << classname() << "::step done" << std::endl;
}

// -----------------------------------------------------------------------------

void Model::finalize(State & xx) const {
  oops::Log::trace() << classname() << "::finalize starting" << std::endl;

  model_->finalize(xx);

  oops::Log::trace() << classname() << "::finalize done" << std::endl;
}

// -----------------------------------------------------------------------------

void Model::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << *model_;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
