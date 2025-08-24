/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "eckit/config/Configuration.h"

#include "ioda/ObsSpace.h"
#include "ioda/ObsVector.h"

#include "oops/base/ObsLocalizationBase.h"

#include "vind/Traits.h"

#include "ufo/ObsTraits.h"

namespace vind {
  class GeometryIterator;

// -----------------------------------------------------------------------------

class ObsLocalization: public oops::ObsLocalizationBase<Traits, ufo::ObsTraits> {
 public:
  ObsLocalization(const eckit::Configuration &,
                  const ioda::ObsSpace &);

 protected:
  void computeLocalization(const GeometryIterator &,
                           ioda::ObsVector &) const override;

 private:
  void print(std::ostream &) const override;

  // Localization function
  double locFunc(const double &) const;

  // Observations coordinates
  std::vector<float> obsLon_;
  std::vector<float> obsLat_;
  std::vector<float> obsHeight_;

  // Localization function and scales
  const std::string locFunc_;
  const double horScale_;
  const double verScale_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
