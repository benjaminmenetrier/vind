/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "vind/ObsLocalization.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "eckit/config/Configuration.h"
#include "eckit/geometry/Point3.h"

#include "oops/generic/gc99.h"

#include "vind/GeometryIterator.h"

// -----------------------------------------------------------------------------

namespace vind {

// -----------------------------------------------------------------------------

static oops::ObsLocalizationMaker<Traits, ufo::ObsTraits, ObsLocalization>
  makerObsLocalization_("default");

// -----------------------------------------------------------------------------

ObsLocalization::ObsLocalization(const eckit::Configuration & config,
                                 const ioda::ObsSpace & obsSpace)
  : obsLon_(obsSpace.nlocs()), obsLat_(obsSpace.nlocs()),  obsHeight_(obsSpace.nlocs()),
  locFunc_(config.getString("localization function", "gc99")),
  horScale_(config.getDouble("horizontal length-scale", 0.0)),
  verScale_(config.getDouble("vertical length-scale", 0.0)) {
  oops::Log::trace() << "ObsLocalization::ObsLocalization starting" << std::endl;

  // Read observations coordinates
  obsSpace.get_db("MetaData", "longitude", obsLon_);
  obsSpace.get_db("MetaData", "latitude", obsLat_);
  if (obsSpace.has("MetaData", "height")) {
    obsSpace.get_db("MetaData", "height", obsHeight_);
  } else {
    obsHeight_.resize(obsLon_.size());
    std::fill(obsLon_.begin(), obsLon_.end(), 0.0);
  }

  oops::Log::trace() << "ObsLocalization::ObsLocalization done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsLocalization::computeLocalization(const GeometryIterator & geometryIterator,
                                          ioda::ObsVector & obsVector) const {
  oops::Log::trace() << "ObsLocalization::computeLocalization starting" << std::endl;

  // Get grid point coordinates
  eckit::geometry::Point3 gridPoint = *geometryIterator;
  const atlas::PointLonLat horGridPoint({gridPoint[0], gridPoint[1]});

  // Loop over observations
  const size_t nvars = obsVector.nvars();
  const double missing = util::missingValue<double>();
  for (size_t jloc = 0; jloc < obsVector.nlocs(); ++jloc) {
    // Compute normalized horizontal distance
    const atlas::PointLonLat horObsPoint({obsLon_[jloc], obsLat_[jloc]});
    double horDist = atlas::util::Earth().distance(horGridPoint, horObsPoint);
    if (horDist > 0.0) {
      if (horScale_ > 0.0) {
        horDist /= horScale_;
      } else {
        horDist = 1.0;
      }
    }

    // Compute normalized vertical distance
    double verDist = 0.0;
    if (geometryIterator.iteratorDimension() == 3) {
      verDist = std::abs(gridPoint[2] - obsHeight_[jloc]);
    }
    if (verDist > 0.0) {
      if (verScale_ > 0.0) {
        verDist /= verScale_;
      } else {
        verDist = 1.0;
      }
    }

    if ((horDist < 1.0) && (verDist < 1.0)) {
      // Compute localization as a product of horizontal and vertical components
      const double loc = locFunc(horDist)*locFunc(verDist);
      for (size_t jvar = 0; jvar < nvars; ++jvar) {
        if (obsVector[jvar+jloc*nvars] != missing) {
          obsVector[jvar+jloc*nvars] *= loc;
        }
      }
    } else {
      // Set at missing value
      for (size_t jvar = 0; jvar < nvars; ++jvar) {
        obsVector[jvar+jloc*nvars] = missing;
      }
    }
  }

  oops::Log::trace() << "ObsLocalization::computeLocalization done" << std::endl;
}

// -----------------------------------------------------------------------------

double ObsLocalization::locFunc(const double & normDist) const {
  double res = 0.0;
  if (locFunc_ == "gc99") {
    res = oops::gc99(normDist);
  } else {
    throw eckit::UserError("Wrong localization function", Here());
  }

  return res;
}

// -----------------------------------------------------------------------------

void ObsLocalization::print(std::ostream & os) const {
  os << "ObsLocalization with length-scale: " << horScale_ << " / " << verScale_ << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
