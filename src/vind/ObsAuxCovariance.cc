/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/ObsAuxCovariance.h"

#include <cmath>
#include <iostream>
#include <random>
#include <string>

#include "eckit/config/Configuration.h"
#include "eckit/exception/Exceptions.h"

#include "oops/util/Logger.h"

#include "vind/ObsAuxIncrement.h"

namespace vind {

// -----------------------------------------------------------------------------

static oops::ObsAuxCovarianceMaker<Traits, ObsAuxCovariance> makeObsAuxCovariance_("ObsAuxControl");

// -----------------------------------------------------------------------------

ObsAuxCovariance::ObsAuxCovariance(const ObsSpace &,
                                   const eckit::Configuration & conf)
  : conf_(conf), variance_(0.0), active_(false), key_(0) {
  oops::Log::trace() << classname() << "::ObsAuxCovariance starting" << std::endl;

  const double zz = conf_.getDouble("standard deviation", 1.0);
  variance_ = zz * zz;
  ASSERT(variance_ > 0.0);
  oops::Log::info() << "ObsAuxCovariance variance = " << variance_ << std::endl;

  oops::Log::trace() << classname() << "::ObsAuxCovariance done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsAuxCovariance::multiply(const oops::ObsAuxIncrementBase<Traits> & dxin,
                                oops::ObsAuxIncrementBase<Traits> & dxout) const {
  oops::Log::trace() << classname() << "::multiply starting" << std::endl;

  dxout = dxin;
  dxout *= variance_;

  oops::Log::trace() << classname() << "::multiply starting" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsAuxCovariance::inverseMultiply(const oops::ObsAuxIncrementBase<Traits> & dxin,
                                       oops::ObsAuxIncrementBase<Traits> & dxout) const {
  oops::Log::trace() << classname() << "::inverseMultiply starting" << std::endl;

  dxout = dxin;
  dxout *= 1.0 / variance_;

  oops::Log::trace() << classname() << "::inverseMultiply done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsAuxCovariance::randomize(oops::ObsAuxIncrementBase<Traits> & dx) const {
  oops::Log::trace() << classname() << "::randomize starting" << std::endl;

  ObsAuxIncrement * pdx = dynamic_cast<ObsAuxIncrement*>(&dx);
  ASSERT(pdx != nullptr);
  static std::mt19937 generator(4);
  static std::normal_distribution<double> distribution(0.0, 1.0);
  double zz = distribution(generator);
  (*pdx).value() = zz * std::sqrt(variance_);

  oops::Log::trace() << classname() << "::randomize done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsAuxCovariance::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "ObsAuxCovariance: variance = " << variance_;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
