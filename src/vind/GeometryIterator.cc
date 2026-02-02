/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "vind/GeometryIterator.h"

#include <vector>

#include "vind/Geometry.h"

// -----------------------------------------------------------------------------

namespace vind {

// -----------------------------------------------------------------------------

GeometryIterator::GeometryIterator(const GeometryIterator & other)
  : geom_(other.geom_), iteratorDimension_(other.iteratorDimension_), jnode_(other.jnode_),
  jlevel_(other.jlevel_) {}

// -----------------------------------------------------------------------------

GeometryIterator::GeometryIterator(const Geometry & geom,
                                   const int & jnode,
                                   const int & jlevel)
  : geom_(geom), iteratorDimension_(geom.iteratorDimension()), jnode_(jnode), jlevel_(jlevel) {}

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
    const auto vcView = atlas::array::make_view<double, 2>(geom_.fields().field(
      geom_.commonVerticalCoordinate()));
    return eckit::geometry::Point3(lonLatView(jnode_, 0), lonLatView(jnode_, 1),
      vcView(jnode_, jlevel_));
  }
}

// -----------------------------------------------------------------------------

GeometryIterator& GeometryIterator::operator++() {
  const auto ownedView = atlas::array::make_view<int, 2>(geom_.fields().field("owned"));
  bool ownedPoint = false;
  do {
    // Increment index
    ++jnode_;

    if (jnode_ == static_cast<int>(geom_.nnodes())) {
      if (geom_.iteratorDimension() == 2) {
        // End of the 2D grid
        jnode_ = -1;
        jlevel_ = -1;
        return *this;
      } else {
        // Next level of the 3D grid
        ++jlevel_;
        jnode_ = 0;
        if (jlevel_ == static_cast<int>(geom_.nlevs())) {
          // End of the 3D grid
          jnode_ = -1;
          jlevel_ = -1;
          return *this;
        }
      }
    }

    // Check if the point is owned by this task
    ownedPoint = (ownedView(jnode_, 0) == 1);
  } while (!ownedPoint);
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
