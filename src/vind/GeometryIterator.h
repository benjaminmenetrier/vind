/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <iterator>
#include <string>
#include <vector>

#include "eckit/geometry/Point3.h"

#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vind/Geometry.h"

namespace vind {

// -----------------------------------------------------------------------------
/// GeometryIterator class

class GeometryIterator: public util::Printable,
                        private util::ObjectCounter<GeometryIterator> {
 public:
  typedef std::forward_iterator_tag iterator_category;
  typedef eckit::geometry::Point3 value_type;
  typedef ptrdiff_t difference_type;
  typedef eckit::geometry::Point3& reference;
  typedef eckit::geometry::Point3* pointer;

  static const std::string classname() {return "vind::GeometryIterator";}

  GeometryIterator(const GeometryIterator & other)
    : geom_(other.geom_), iteratorDimension_(other.iteratorDimension_), jnode_(other.jnode_),
    jlevel_(other.jlevel_) {}
  GeometryIterator(const Geometry & geom,
                   const size_t & jnode,
                   const size_t & jlevel)
    : geom_(geom), iteratorDimension_(geom.iteratorDimension()), jnode_(jnode), jlevel_(jlevel) {}
  ~GeometryIterator() {}

  bool operator==(const GeometryIterator &) const;
  bool operator!=(const GeometryIterator &) const;
  eckit::geometry::Point3 operator*() const;
  GeometryIterator& operator++();

  const size_t & iteratorDimension() const
    {return iteratorDimension_;}
  const size_t & jnode() const
    {return jnode_;}
  const size_t & jlevel() const
    {return jlevel_;}

 private:
  void print(std::ostream & os) const override;

  const  Geometry & geom_;
  size_t iteratorDimension_;
  size_t jnode_;
  size_t jlevel_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
