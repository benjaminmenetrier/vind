/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "vind/GeometryIterator.h"

#include <vector>

// -----------------------------------------------------------------------------

namespace vind {

// -----------------------------------------------------------------------------

bool GeometryIterator::operator==(const GeometryIterator & other) const {
  // TODO(Benjamin): check geometry consistency
  return ((jnode_ == other.jnode_) && (jlevel_ == other.jlevel_));
}

// -----------------------------------------------------------------------------

bool GeometryIterator::operator!=(const GeometryIterator & other) const {
  // TODO(Benjamin): check geometry consistency
  return ((jnode_ != other.jnode_) || (jlevel_ != other.jlevel_));
}

// -----------------------------------------------------------------------------

eckit::geometry::Point3 GeometryIterator::operator*() const {
  // Get lon/lat and vertical coordinate fields from geometry
  const auto lonLatView = atlas::array::make_view<double, 2>(geom_.functionSpace().lonlat());
  if (geom_.iteratorDimension() == 2) {
    return eckit::geometry::Point3(lonLatView(jnode_, 0), lonLatView(jnode_, 1), 0.0);
  } else {
    const auto vcView = atlas::array::make_view<double, 2>(geom_.fields().field("vert_coord_0"));
    return eckit::geometry::Point3(lonLatView(jnode_, 0), lonLatView(jnode_, 1),
      vcView(jnode_, jlevel_));
  }
}

// -----------------------------------------------------------------------------

GeometryIterator& GeometryIterator::operator++() {
  const auto ownedView = atlas::array::make_view<int, 2>(geom_.fields().field("owned"));
  bool ownedPoint = false;
  do {
    ++jnode_;
    if (jnode_ < geom_.nnodes()) {
      ownedPoint = (ownedView(jnode_, 0) == 1);
    } else {
      ownedPoint = true;
    }
  } while (!ownedPoint);
  if (jnode_ == geom_.nnodes()) {
    // End of horizontal counter
    if (geom_.iteratorDimension() == 2) {
      jlevel_ = geom_.nlevs();
    } else {
      ++jlevel_;
      if (jlevel_ < geom_.nlevs()) {
        jnode_ = 0;
        do {
          ++jnode_;
        } while (ownedView(jnode_, 0) == 0);
      }
    }
  }
  return *this;
}

// -----------------------------------------------------------------------------

void GeometryIterator::print(std::ostream & os) const {
  if (geom_.iteratorDimension() == 2) {
    os << "GeometryIterator: (" << jnode_ << ")";
  } else {
    os << "GeometryIterator: (" << jnode_ << "," << jlevel_ << ")";
  }
}

// -----------------------------------------------------------------------------

}  // namespace vind
