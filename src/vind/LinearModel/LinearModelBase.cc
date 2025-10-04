/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/LinearModel/LinearModelBase.h"

#include <map>
#include <memory>
#include <string>

#include "eckit/config/Configuration.h"

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Increment.h"

namespace vind {

// -----------------------------------------------------------------------------

LinearModelFactory::LinearModelFactory(const std::string & name) {
  if (getMakers().find(name) != getMakers().end()) {
    throw std::runtime_error(name + " already registered in the model factory.");
  }
  getMakers()[name] = this;
}

// -----------------------------------------------------------------------------

LinearModelBase * LinearModelFactory::create(const Geometry & geom,
                                             const eckit::Configuration & config) {
  oops::Log::trace() << "LinearModelFactory::create starting" << std::endl;
  const std::string id = config.getString("name");
  typename std::map<std::string, LinearModelFactory*>::iterator imodel = getMakers().find(id);
  if (imodel == getMakers().end()) {
    throw std::runtime_error(id + " does not exist in the model factory");
  }
  LinearModelBase * ptr = imodel->second->make(geom, config);
  oops::Log::trace() << "LinearModelFactory::create done" << std::endl;
  return ptr;
}

// -----------------------------------------------------------------------------

}  // namespace vind
