/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/GeoVaLs.h"

#include "eckit/exception/Exceptions.h"

#include "oops/util/FieldSetHelpers.h"
#include "oops/util/FieldSetOperations.h"
#include "oops/util/Logger.h"
#include "oops/util/RandomField.h"

#include "vind/ObsSpace.h"
#include "vind/Utilities.h"

namespace vind {

// -----------------------------------------------------------------------------

GeoVaLs::GeoVaLs(const ObsSpace & obsSpace,
                 const varns::Variables & vars,
                 const Increment &,
                 const util::DateTime & t1,
                 const util::DateTime & t2)
  : comm_(obsSpace.getComm()), obsSpace_(obsSpace), iobs_(), gvFieldSet_() {
  oops::Log::trace() << classname() << "::GeoVaLs starting" << std::endl;

  // Active observations indices
  iobs_ = obsSpace_.timeSelect(t1, t2);

  // Create GeoVaLs fieldset
  for (const auto & var : vars.variables()) {
    atlas::Field gvField(var, atlas::array::make_datatype<double>(),
      atlas::array::make_shape(iobs_.size(), 1));
    gvFieldSet_.add(gvField);
  }

  oops::Log::trace() << classname() << "::GeoVaLs done" << std::endl;
}

// -----------------------------------------------------------------------------

GeoVaLs::GeoVaLs(const ObsSpace & obsSpace,
                 const varns::Variables & vars,
                 const State &,
                 const util::DateTime & t1,
                 const util::DateTime & t2)
  : comm_(obsSpace.getComm()), obsSpace_(obsSpace), iobs_(), gvFieldSet_() {
  oops::Log::trace() << classname() << "::GeoVaLs starting" << std::endl;

  // Active observations indices
  iobs_ = obsSpace_.timeSelect(t1, t2);

  // Create GeoVaLs fieldset
  for (const auto & var : vars.variables()) {
    atlas::Field gvField(var, atlas::array::make_datatype<double>(),
      atlas::array::make_shape(iobs_.size(), 1));
    gvFieldSet_.add(gvField);
  }

  oops::Log::trace() << classname() << "::GeoVaLs done" << std::endl;
}

// -----------------------------------------------------------------------------

void GeoVaLs::zero() {
  oops::Log::trace() << classname() << "::zero starting" << std::endl;

  util::zeroFieldSet(gvFieldSet_);

  oops::Log::trace() << classname() << "::zero done" << std::endl;
}

// -----------------------------------------------------------------------------

void GeoVaLs::random() {
  oops::Log::trace() << classname() << "::random starting" << std::endl;

  throw eckit::NotImplemented("no MPI-safe random generator yet", Here());

  oops::Log::trace() << classname() << "::random done" << std::endl;
}

// -----------------------------------------------------------------------------

double GeoVaLs::dot_product_with(const GeoVaLs & other) const {
  oops::Log::trace() << classname() << "::dot_product_with starting" << std::endl;

  double zz = dotProductFieldSetsWithoutFunctionSpace(gvFieldSet_, other.gvFieldSet_,
    gvFieldSet_.field_names(), comm_);

  oops::Log::trace() << classname() << "::dot_product_with done" << std::endl;
  return zz;
}

// -----------------------------------------------------------------------------

void GeoVaLs::read(const eckit::Configuration & conf) {
  oops::Log::trace() << classname() << "::read starting " << std::endl;

  throw eckit::NotImplemented("no MPI-safe read yet", Here());

  oops::Log::trace() << classname() << "::read done" << std::endl;
}

// -----------------------------------------------------------------------------

void GeoVaLs::write(const eckit::Configuration & conf) const {
  oops::Log::trace() << classname() << "::write starting" << std::endl;

  throw eckit::NotImplemented("no MPI-safe write yet", Here());

  oops::Log::trace() << classname() << "::write done" << std::endl;
}

// -----------------------------------------------------------------------------

void GeoVaLs::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  throw eckit::NotImplemented("no MPI-safe write yet", Here());

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
