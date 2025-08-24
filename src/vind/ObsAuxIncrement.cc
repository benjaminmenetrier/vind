/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/ObsAuxIncrement.h"

#include <iostream>
#include <string>

#include "eckit/config/Configuration.h"
#include "eckit/exception/Exceptions.h"

#include "oops/util/Logger.h"

#include "vind/ObsAuxControl.h"
#include "vind/ObsAuxCovariance.h"
#include "vind/ObsSpace.h"

namespace vind {

// -----------------------------------------------------------------------------

static oops::ObsAuxIncrementMaker<Traits, ObsAuxIncrement> makerObsAuxIncrementDefault_("default");

// -----------------------------------------------------------------------------

ObsAuxIncrement::ObsAuxIncrement(const ObsSpace & obsSpace,
                                 const eckit::Configuration & conf)
  : comm_(obsSpace.getComm()), bias_(0.0), active_(false), dummyKey_(0) {
  oops::Log::trace() << classname() << "::ObsAuxIncrement starting" << std::endl;

  active_ = conf.has("standard_deviation");

  oops::Log::trace() << classname() << "::ObsAuxIncrement done" << std::endl;
}

// -----------------------------------------------------------------------------

ObsAuxIncrement::ObsAuxIncrement(const ObsAuxIncrement & other,
                                 const bool copy)
  : comm_(other.comm_), bias_(0.0), active_(other.active_), dummyKey_(other.dummyKey_) {
  oops::Log::trace() << classname() << "::ObsAuxIncrement starting" << std::endl;

  if (active_ && copy) {
    bias_ = other.bias_;
  }

  oops::Log::trace() << classname() << "::ObsAuxIncrement done" << std::endl;
}

// -----------------------------------------------------------------------------

ObsAuxIncrement::ObsAuxIncrement(const ObsAuxIncrement & other,
                                 const eckit::Configuration &)
  : comm_(other.comm_), bias_(0.0), active_(other.active_) {
  oops::Log::trace() << classname() << "::ObsAuxIncrement starting" << std::endl;

  if (active_) {
    bias_ = other.bias_;
  }

  oops::Log::trace() << classname() << "::ObsAuxIncrement done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsAuxIncrement::diff(const oops::ObsAuxControlBase<Traits> & b1,
                           const oops::ObsAuxControlBase<Traits> & b2) {
  oops::Log::trace() << classname() << "::diff starting" << std::endl;

  const ObsAuxControl * pb1 = dynamic_cast<const ObsAuxControl*>(&b1);
  const ObsAuxControl * pb2 = dynamic_cast<const ObsAuxControl*>(&b2);
  ASSERT(pb1 != nullptr && pb2 != nullptr);
  if (active_) {
    bias_ = (*pb1).value() - (*pb2).value();
  }

  oops::Log::trace() << classname() << "::diff done" << std::endl;
}

// -----------------------------------------------------------------------------

ObsAuxIncrement & ObsAuxIncrement::operator=(const oops::ObsAuxIncrementBase<Traits> & rhs) {
  oops::Log::trace() << classname() << "::operator= starting" << std::endl;

  if (active_) {
    const ObsAuxIncrement * prhs = dynamic_cast<const ObsAuxIncrement*>(&rhs);
    ASSERT(prhs != nullptr);
    bias_ = (*prhs).value();
  } else {
    bias_ = 0.0;
  }

  oops::Log::trace() << classname() << "::operator= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

ObsAuxIncrement & ObsAuxIncrement::operator=(const ObsAuxIncrement & rhs) {
  oops::Log::trace() << classname() << "::operator= starting" << std::endl;

  if (active_) {
    bias_ = rhs.bias_;
  } else {
    bias_ = 0.0;
  }

  oops::Log::trace() << classname() << "::operator= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

ObsAuxIncrement & ObsAuxIncrement::operator+=(const oops::ObsAuxIncrementBase<Traits> & rhs) {
  oops::Log::trace() << classname() << "::operator+= starting" << std::endl;

  if (active_) {
    const ObsAuxIncrement * prhs = dynamic_cast<const ObsAuxIncrement*>(&rhs);
    ASSERT(prhs != nullptr);
    bias_ += (*prhs).value();
  }

  oops::Log::trace() << classname() << "::operator+= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

ObsAuxIncrement & ObsAuxIncrement::operator-=(const oops::ObsAuxIncrementBase<Traits> & rhs) {
  oops::Log::trace() << classname() << "::operator-= starting" << std::endl;

  if (active_) {
    const ObsAuxIncrement * prhs = dynamic_cast<const ObsAuxIncrement*>(&rhs);
    ASSERT(prhs != nullptr);
    bias_ -= (*prhs).value();
  }

  oops::Log::trace() << classname() << "::operator-= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

ObsAuxIncrement & ObsAuxIncrement::operator*=(const double & fact) {
  oops::Log::trace() << classname() << "::operator*= starting" << std::endl;

  if (active_) {
    bias_ *= fact;
  }

  oops::Log::trace() << classname() << "::operator*= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

void ObsAuxIncrement::axpy(const double & fact,
                           const oops::ObsAuxIncrementBase<Traits> & rhs) {
  oops::Log::trace() << classname() << "::axpy starting" << std::endl;

  if (active_) {
    const ObsAuxIncrement * prhs = dynamic_cast<const ObsAuxIncrement*>(&rhs);
    ASSERT(prhs != nullptr);
    bias_ += fact * (*prhs).value();
  }

  oops::Log::trace() << classname() << "::axpy done" << std::endl;
}

// -----------------------------------------------------------------------------

double ObsAuxIncrement::dot_product_with(const oops::ObsAuxIncrementBase<Traits> & rhs) const {
  oops::Log::trace() << classname() << "::dot_product_with starting" << std::endl;

  double zz = 0.0;
  if (active_) {
    const ObsAuxIncrement * prhs = dynamic_cast<const ObsAuxIncrement*>(&rhs);
    ASSERT(prhs != nullptr);
    zz = bias_ * (*prhs).value();
  }
  comm_.allReduceInPlace(zz, eckit::mpi::sum());

  oops::Log::trace() << classname() << "::dot_product_with done" << std::endl;
  return zz;
}

// -----------------------------------------------------------------------------

void ObsAuxIncrement::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  if (active_) {
    os << std::endl << "ObsAuxIncrement = " << bias_;
  }

  oops::Log::trace() << classname() << "::print end" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
