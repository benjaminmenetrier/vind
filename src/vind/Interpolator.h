/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <memory>
#include <ostream>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/NonCopyable.h"

#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class State;
  class Increment;
  class Variables;

// -----------------------------------------------------------------------------
/// Interpolator class

class Interpolator: public util::Printable,
                    private eckit::NonCopyable,
                    private util::ObjectCounter<Interpolator> {
 public:
  static const std::string classname()
    {return "vind::Interpolator";}

/// OOPS interface

// Constructors/destructor
  Interpolator(const State &,
               const eckit::Configuration &,
               const bool)
    {throw eckit::NotImplemented(Here());}
  Interpolator(const State &,
               const Geometry &,
               const eckit::Configuration &,
               const bool)
    {throw eckit::NotImplemented(Here());}
  explicit Interpolator(const Interpolator &)
    {throw eckit::NotImplemented(Here());}
  ~Interpolator()
    {}

// Basic operators
  Interpolator & operator=(const Interpolator &)
    {throw eckit::NotImplemented(Here()); return *this;}
  Interpolator & operator+=(const State &)
    {throw eckit::NotImplemented(Here()); return *this;}
  Interpolator & operator-=(const State &)
    {throw eckit::NotImplemented(Here()); return *this;}
  void set(const Geometry &,
           const Geometry &)
    {throw eckit::NotImplemented(Here());}
  virtual void set_vector(const eckit::Configuration &)
    {throw eckit::NotImplemented(Here());}

// Interpolate/smooth
  void interpolate(const Increment &,
                   const Variables &,
                   Increment &) const
    {throw eckit::NotImplemented(Here());}
  void interpolate(const State &,
                   const Variables &,
                   State &) const
    {throw eckit::NotImplemented(Here());}
  void smooth(Increment &) const
    {throw eckit::NotImplemented(Here());}

// Save/read/write
  void save(Increment &) const
    {throw eckit::NotImplemented(Here());}
  void read(const Increment &)
    {throw eckit::NotImplemented(Here());}
  void write(const eckit::Configuration &) const
    {throw eckit::NotImplemented(Here());}

 private:
  void print(std::ostream &) const
    {throw eckit::NotImplemented(Here());}
};

// -----------------------------------------------------------------------------

}  // namespace vind
