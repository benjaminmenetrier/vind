/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/Variables.h"

#include "eckit/config/Configuration.h"

#include "oops/util/ConfigFunctions.h"
#include "oops/util/Logger.h"

namespace vind {

// -----------------------------------------------------------------------------

Variables::Variables(const eckit::Configuration & config)
  : oops::JediVariables() {
  oops::Log::trace() << classname() << "::Variables starting" << std::endl;

  // Get variables names
  std::vector<std::string> varNames;
  if (util::isVector(config)) {
    varNames = config.getStringVector(".");
  } else if (config.has("variables")) {
    varNames = config.getStringVector("variables");
  } else if (config.has("variables list")) {
    varNames = config.getStringVector("variables list");
  } else {
    oops::Log::info() << "Configuration passed to Variables: " << config << std::endl;
    throw eckit::Exception("wrong variables configuration", Here());
  }

  // Add variables
  for (const std::string & varName : varNames) {
    this->push_back(varName);
  }

  oops::Log::trace() << classname() << "::Variables done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
