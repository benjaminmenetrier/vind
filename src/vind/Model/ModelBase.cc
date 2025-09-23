/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/Model/ModelBase.h"

#include <map>
#include <memory>
#include <string>

#include "eckit/config/Configuration.h"

#include "oops/util/Logger.h"

namespace vind {

// -----------------------------------------------------------------------------

ModelFactory::ModelFactory(const std::string & name) {
  if (getMakers().find(name) != getMakers().end()) {
    throw std::runtime_error(name + " already registered in the model factory.");
  }
  getMakers()[name] = this;
}

// -----------------------------------------------------------------------------

ModelBase * ModelFactory::create(const Geometry & geom,
                                 const eckit::Configuration & config) {
  oops::Log::trace() << "ModelFactory::create starting" << std::endl;
  const std::string id = config.getString("name");
  typename std::map<std::string, ModelFactory*>::iterator imodel = getMakers().find(id);
  if (imodel == getMakers().end()) {
    throw std::runtime_error(id + " does not exist in the model factory");
  }
  ModelBase * ptr = imodel->second->make(geom, config);
  oops::Log::trace() << "ModelFactory::create done" << std::endl;
  return ptr;
}

// -----------------------------------------------------------------------------

}  // namespace vind
