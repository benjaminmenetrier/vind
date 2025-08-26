/*
 * (C) Copyright 2022 UCAR.
 * (C) Copyright 2023-2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "vind/Increment.h"

#include <vector>

#include "atlas/field.h"

#include "oops/util/Logger.h"

namespace vind {

// -----------------------------------------------------------------------------

Increment::Increment(const Geometry & geom,
                     const oops::Variables & vars,
                     const util::DateTime & vt)
  : fields_(new Fields(geom, vars, vt, false)) {
  oops::Log::trace() << classname() << "::Increment starting" << std::endl;

  fields_->zero();

  oops::Log::trace() << classname() << "::Increment done" << std::endl;
}

// -----------------------------------------------------------------------------

Increment::Increment(const Geometry & geom,
                     const Increment & other)
  : fields_(new Fields(*other.fields_, geom)) {
  oops::Log::trace() << classname() << "::Increment starting" << std::endl;

  fields_->zero();

  oops::Log::trace() << classname() << "::Increment done" << std::endl;
}

// -----------------------------------------------------------------------------

Increment::Increment(const Increment & other,
                     const bool copy)
  : fields_(new Fields(*other.fields_, copy)) {
  oops::Log::trace() << classname() << "::Increment" << std::endl;
}

// -----------------------------------------------------------------------------

void Increment::diff(const State & x1,
                     const State & x2) {
  oops::Log::trace() << classname() << "::diff starting" << std::endl;

  ASSERT(this->validTime() == x1.validTime());
  ASSERT(this->validTime() == x2.validTime());
  fields_->diff(x1.fields(), x2.fields());

  oops::Log::trace() << classname() << "::diff done" << std::endl;
}

// -----------------------------------------------------------------------------

Increment & Increment::operator=(const Increment & rhs) {
  oops::Log::trace() << classname() << "::operator= starting" << std::endl;

  fields_.reset(new Fields(*rhs.fields_));

  oops::Log::trace() << classname() << "::operator= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

Increment & Increment::operator+=(const Increment & dx) {
  oops::Log::trace() << classname() << "::operator+= starting" << std::endl;

  ASSERT(this->validTime() == dx.validTime());
  *fields_ += *dx.fields_;

  oops::Log::trace() << classname() << "::operator+= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

Increment & Increment::operator-=(const Increment & dx) {
  oops::Log::trace() << classname() << "::operator-= starting" << std::endl;

  ASSERT(this->validTime() == dx.validTime());
  *fields_ -= *dx.fields_;

  oops::Log::trace() << classname() << "::operator-= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

Increment & Increment::operator*=(const double & zz) {
  oops::Log::trace() << classname() << "::operator*= starting" << std::endl;

  *fields_ *= zz;

  oops::Log::trace() << classname() << "::operator*= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

void Increment::zero(const util::DateTime & vt) {
  oops::Log::trace() << classname() << "::zero starting" << std::endl;

  fields_->zero();
  fields_->time() = vt;

  oops::Log::trace() << classname() << "::zero done" << std::endl;
}

// -----------------------------------------------------------------------------

void Increment::axpy(const double & zz,
                     const Increment & dx,
                     const bool check) {
  oops::Log::trace() << classname() << "::axpy starting" << std::endl;

  ASSERT(!check || this->validTime() == dx.validTime());
  fields_->axpy(zz, *dx.fields_);

  oops::Log::trace() << classname() << "::axpy done" << std::endl;
}

// -----------------------------------------------------------------------------

oops::LocalIncrement Increment::getLocal(const GeometryIterator & geometryIterator) const {
  int index = 0;
  if (this->geometry()->iteratorDimension() == 2) {
    std::vector<int> variableSizes;
    for (const auto & var : this->variables()) {
      variableSizes.push_back(static_cast<int>(var.getLevels()));
    }
    size_t valuesSize = std::accumulate(variableSizes.begin(), variableSizes.end(), 0);
    std::vector<double> values(valuesSize);
    for (const auto & var : this->variables()) {
      const auto view = atlas::array::make_view<double, 2>(this->fields().fieldSet()[var.name()]);
      for (size_t jlevel = 0; jlevel < var.getLevels(); ++jlevel) {
        values[index] = view(geometryIterator.jnode(), jlevel);
        ++index;
      }
    }
    return oops::LocalIncrement(this->variables(), values, variableSizes);
  } else {
    std::vector<int> variableSizes(this->variables().size(), 1);
    size_t valuesSize = this->variables().size();
    std::vector<double> values(valuesSize);
    for (const auto & var : this->variables()) {
      const auto view = atlas::array::make_view<double, 2>(this->fields().fieldSet()[var.name()]);
      values[index] = view(geometryIterator.jnode(), geometryIterator.jlevel());
      ++index;
    }
    return oops::LocalIncrement(this->variables(), values, variableSizes);
  }
}

// -----------------------------------------------------------------------------

void Increment::setLocal(const oops::LocalIncrement & localIncrement,
                         const GeometryIterator & geometryIterator) {
  std::vector<double> values = localIncrement.getVals();
  size_t index = 0;
  if (this->geometry()->iteratorDimension() == 2) {
    for (const auto & var : this->variables()) {
      auto view = atlas::array::make_view<double, 2>(this->fields().fieldSet()[var.name()]);
      for (size_t jlevel = 0; jlevel < var.getLevels(); ++jlevel) {
        view(geometryIterator.jnode(), jlevel) = values[index];
        ++index;
      }
    }
  } else {
    for (const auto & var : this->variables()) {
      auto view = atlas::array::make_view<double, 2>(this->fields().fieldSet()[var.name()]);
      view(geometryIterator.jnode(), geometryIterator.jlevel()) = values[index];
      ++index;
    }
  }
}

// -----------------------------------------------------------------------------

void Increment::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << std::endl << "- Valid time: " << this->validTime();
  os << *fields_;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
