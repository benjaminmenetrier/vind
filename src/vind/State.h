/*
 * (C) Copyright 2022 UCAR.
 * (C) Copyright 2023-2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"

#include "oops/base/Variables.h"
#include "oops/util/DateTime.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vind/Fields.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class Increment;

// -----------------------------------------------------------------------------
/// State class

class State : public util::Printable,
              private util::ObjectCounter<State> {
 public:
  static const std::string classname()
    {return "vind::State";}

  // Constructors
  State(const Geometry &,
        const oops::Variables &,
        const util::DateTime &);
  State(const Geometry &,
        const eckit::Configuration &);
  State(const Geometry & geom,
        const State & other)
    : fields_(new Fields(*other.fields_, geom)) {}
  State(const oops::Variables & vars,
        const State & other)
    : fields_(new Fields(*other.fields_)) {}
  State(const State & other)
    : fields_(new Fields(*other.fields_)) {}

  // Assignment
  State & operator=(const State &);

  // Interactions with Increment
  State & operator+=(const Increment &);

  // I/O and diagnostics
  void read(const eckit::Configuration & config)
    {fields_->read(config);}
  void write(const eckit::Configuration & config) const
    {fields_->write(config);}
  double norm() const
    {return fields_->norm();}
  const util::DateTime & validTime() const
    {return fields_->validTime();}
  void updateTime(const util::Duration & dt)
    {fields_->updateTime(dt);}

  // Access to fields
  Fields & fields()
    {return *fields_;}
  const Fields & fields() const
    {return *fields_;}

  // ATLAS FieldSet accessor
  void toFieldSet(atlas::FieldSet & fset) const
    {fields_->toFieldSet(fset);}
  void fromFieldSet(const atlas::FieldSet & fset)
    {fields_->fromFieldSet(fset);}
  const atlas::FieldSet & fieldSet() const
    {return fields_->fieldSet();}
  atlas::FieldSet & fieldSet()
    {return fields_->fieldSet();}
  void synchronizeFields()
    {fields_->synchronizeFields();}

  // Other
  void zero()
    {fields_->zero();}
  void accumul(const double & zz,
               const State & xx)
    {fields_->axpy(zz, xx.fields());}
  const oops::Variables & variables() const
    {return fields_->variables();}

  // Serialization
  size_t serialSize() const
    {return fields_->serialSize();}
  void serialize(std::vector<double> & vect) const
    {fields_->serialize(vect);}
  void deserialize(const std::vector<double> & vect,
                   size_t & index)
    {fields_->deserialize(vect, index);}
  void transpose(const State &,
                 const eckit::mpi::Comm &,
                 const int,
                 const int)
    {throw eckit::Exception("not implemented yet", Here());}

 private:
  // Print
  void print(std::ostream &) const;

  // Fields
  std::unique_ptr<Fields> fields_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
