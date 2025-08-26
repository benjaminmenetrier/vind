/*
 * (C) Copyright 2024-     UCAR.
 * (C) Copyright 2023-2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "vind/VariableChange.h"

#include <map>
#include <ostream>
#include <string>
#include <typeinfo>
#include <vector>

#include "oops/base/Variables.h"
#include "oops/util/ConfigFunctions.h"
#include "oops/util/FieldSetHelpers.h"
#include "oops/util/FieldSetOperations.h"
#include "oops/util/Logger.h"

#include "vind/Constants.h"
#include "vind/Geometry.h"
#include "vind/ModelData.h"
#include "vind/State.h"
#include "vind/VaderCookbook.h"
#include "vind/VariableChangeParameters.h"

namespace vind {

// -----------------------------------------------------------------------------

VariableChange::VariableChange(const eckit::Configuration & config,
                               const Geometry & geom)
  : geom_(geom), vader_() {
  oops::Log::trace() << classname() << "::VariableChange starting" << std::endl;

  // Deserialize configuration
  VariableChangeParameters params;
  params.deserialize(config);

  const boost::optional<vader::VaderParameters> &vaderParam = params.vaderParam.value();
  if (vaderParam != boost::none) {
    // Pass model data parameters to vader configuration
    ModelData modelData(geom);

    // Add all constants to modelData config
    std::vector<std::string> allConstantsNames = getAllConstantsNames();
    eckit::LocalConfiguration modelDataObject = modelData.modelData();
    for (std::string& ConstantName : allConstantsNames) {
        modelDataObject.set(ConstantName, getConstant(ConstantName));
    }

    eckit::LocalConfiguration vaderConfig;
    vaderConfig.set(vader::configCookbookKey,
      params.toConfiguration().getSubConfiguration("vader custom cookbook"));
    vaderConfig.set(vader::configModelVarsKey, modelDataObject);

    // Create vader with vind custom cookbook
    vader_.reset(new vader::Vader(*vaderParam, vaderConfig));
  }

  oops::Log::trace() << classname() << "::VariableChange done" << std::endl;
}

// -----------------------------------------------------------------------------

void VariableChange::changeVar(State & x,
                               const oops::Variables & vars_out) const {
  oops::Log::trace() << classname() << "::changeVar starting" << std::endl;

  // Create FieldSet
  atlas::FieldSet fset;

  // State to FieldSet
  x.toFieldSet(fset);

  if (vader_) {
    // Call vader
    oops::Variables varsCha(vars_out);
    vader_->changeVar(fset, varsCha);
  } else {
    // Create any fields that do not exist
    for (auto & var : vars_out) {
      if (!fset.has(var.name())) {
        fset.add(geom_.functionSpace().createField<double>(
          atlas::option::name(var.name()) |
          atlas::option::levels(geom_.levels(var.name()))));
      }
    }

    // At this point all the fields are available but no actual variable transform
    // has been completed. If the use case arises they should be added here.
    // For example if x contains winds and vars contain stream function and
    // velocity potential, they should be computed. Fields in x that are not
    // in vars should be removed but this might require changes to saber
    // functionality
  }

  // FieldSet to State
  x.fromFieldSet(fset);

  oops::Log::trace() << classname() << "::changeVar done" << std::endl;
}

// -----------------------------------------------------------------------------

void VariableChange::changeVarInverse(State & x,
                                      const oops::Variables & vars_out) const {
  oops::Log::trace() << classname() << "::changeVarInverse starting" << std::endl;

  if (vader_) {
    // State to FieldSet
    atlas::FieldSet fset;
    x.toFieldSet(fset);

    // Call vader
    oops::Variables varsCha(vars_out);
    vader_->changeVar(fset, varsCha);

    // FieldSet to State
    x.fromFieldSet(fset);
  } else {
    // Not implemented yet
    throw eckit::NotImplemented("changeVarInverse not implemented", Here());
  }

  oops::Log::trace() << classname() << "::changeVarInverse done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
