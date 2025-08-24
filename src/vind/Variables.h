/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"

#include "oops/base/Variables.h"
#include "oops/util/ObjectCounter.h"

namespace eckit {
  class Configuration;
}

namespace vind {

// -----------------------------------------------------------------------------
///  Variables class

class Variables : public oops::JediVariables,
                  private util::ObjectCounter<Variables> {
  using vindVariables = oops::JediVariables;
  using vindVariables::vindVariables;

 public:
  static const std::string classname()
    {return "vind::Variables";}

// Extra constructor
  explicit Variables(const eckit::Configuration &);

// Extra accessor
  std::vector<std::string> variablesList() const
    {return this->variables();}
};

// -----------------------------------------------------------------------------

}  // namespace vind
