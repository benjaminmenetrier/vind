/*
 * (C) Copyright 2023 Meteorologisk Institutt
 *
 */

#include "vind/ObsVector.h"

#include <math.h>

#include <algorithm>
#include <limits>
#include <sstream>

#include "eckit/config/Configuration.h"
#include "eckit/exception/Exceptions.h"

#include "oops/util/missingValues.h"

#include "oops/util/DateTime.h"
#include "oops/util/Duration.h"
#include "oops/util/FieldSetHelpers.h"
#include "oops/util/FieldSetOperations.h"
#include "oops/util/Logger.h"
#include "oops/util/Random.h"

#include "vind/Utilities.h"

namespace vind {

// -----------------------------------------------------------------------------

ObsVector::ObsVector(const ObsSpace & obsSpace)
  : comm_(obsSpace.getComm()), obsSpace_(obsSpace), vars_(obsSpace_.vars()), data_(),
    missing_(util::missingValue<double>()) {
  oops::Log::trace() << classname() << "::ObsVector starting" << std::endl;

  // Allocate FieldSet
  for (size_t jvar = 0; jvar < vars_.size(); ++jvar) {
    atlas::Field field(vars_[jvar].name(), atlas::array::make_datatype<double>(),
      atlas::array::make_shape(obsSpace_.sizeLoc(), 1));
    data_.add(field);
  }

  // Set FieldSet to zero
  zero();

  oops::Log::trace() << classname() << "::ObsVector done" << std::endl;
}

// -----------------------------------------------------------------------------

ObsVector::ObsVector(const ObsVector & other,
                     const bool copy)
  : comm_(other.comm_), obsSpace_(other.obsSpace_), vars_(other.vars_), data_(),
    missing_(util::missingValue<double>()) {
  oops::Log::trace() << classname() << "::ObsVector starting" << std::endl;

  copyFieldSetWithoutFunctionSpace(other.data_, data_);
  if (!copy) {
    zero();
  }

  oops::Log::trace() << classname() << "::ObsVector done" << std::endl;
}

// -----------------------------------------------------------------------------

ObsVector & ObsVector::operator= (const ObsVector & rhs) {
  oops::Log::trace() << classname() << "::operator= starting" << std::endl;

  copyFieldSetWithoutFunctionSpace(rhs.data_, data_);

  oops::Log::trace() << classname() << "::operator= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

ObsVector & ObsVector::operator*= (const double & zz) {
  oops::Log::trace() << classname() << "::operator*= starting" << std::endl;

  util::multiplyFieldSet(data_, zz);

  oops::Log::trace() << classname() << "::operator*= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

ObsVector & ObsVector::operator+= (const ObsVector & rhs) {
  oops::Log::trace() << classname() << "::operator+= starting" << std::endl;

  util::addFieldSets(data_, rhs.data_);

  oops::Log::trace() << classname() << "::operator+= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

ObsVector & ObsVector::operator-= (const ObsVector & rhs) {
  oops::Log::trace() << classname() << "::operator-= starting" << std::endl;

  util::subtractFieldSets(data_, rhs.data_);

  oops::Log::trace() << classname() << "::operator-= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

ObsVector & ObsVector::operator*= (const ObsVector & rhs) {
  oops::Log::trace() << classname() << "::operator*= starting" << std::endl;

  util::multiplyFieldSets(data_, rhs.data_);

  oops::Log::trace() << classname() << "::operator*= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

ObsVector & ObsVector::operator/= (const ObsVector & rhs) {
  oops::Log::trace() << classname() << "::operator/= starting" << std::endl;

  util::divideFieldSets(data_, rhs.data_);

  oops::Log::trace() << classname() << "::operator/= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

void ObsVector::zero() {
  oops::Log::trace() << classname() << "::zero starting" << std::endl;

  util::zeroFieldSet(data_);

  oops::Log::trace() << classname() << "::zero done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsVector::ones() {
  oops::Log::trace() << classname() << "::ones starting" << std::endl;

  for (auto & field : data_) {
    auto view = atlas::array::make_view<double, 2>(field);
    view.assign(1.0);
  }
  data_.set_dirty(false);

  oops::Log::trace() << classname() << "::ones done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsVector::invert() {
  oops::Log::trace() << classname() << "::invert starting" << std::endl;

  atlas::FieldSet tmp;
  copyFieldSetWithoutFunctionSpace(data_, tmp);
  ones();
  util::divideFieldSets(data_, tmp);

  oops::Log::trace() << classname() << "::invert done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsVector::axpy(const double & zz,
                     const ObsVector & rhs) {
  oops::Log::trace() << classname() << "::axpy starting" << std::endl;

  atlas::FieldSet tmp;
  copyFieldSetWithoutFunctionSpace(rhs.data_, tmp);
  util::multiplyFieldSet(tmp, zz);
  util::addFieldSets(data_, tmp);

  oops::Log::trace() << classname() << "::axpy done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsVector::random() {
  oops::Log::trace() << classname() << "::random starting" << std::endl;

  // Global vector
  std::vector<double> randomPertGlb;

  if (comm_.rank() == 0) {
    // Random numbers generator
    util::NormalDistribution<double> x(vars_.size()*obsSpace_.sizeGlbAll(), 0.0, 1.0,
      obsSpace_.getSeed());
    std::vector<double> randomPertTmp(vars_.size()*obsSpace_.sizeGlbAll());
    randomPertTmp = x.data();

    // Reorder perturbations
    randomPertGlb.resize(vars_.size()*obsSpace_.sizeGlb());
    size_t jo = 0;
    for (size_t joAll = 0; joAll < obsSpace_.sizeGlbAll(); ++joAll) {
      if (obsSpace_.maskSum()[joAll] > 0) {
        for (size_t jvar = 0; jvar < vars_.size(); ++jvar) {
          randomPertGlb[jo*vars_.size()+jvar] =
            randomPertTmp[obsSpace_.order()[jo]*vars_.size()+jvar];
        }
        ++jo;
      }
    }
  }

  // Define counts and displacements
  std::vector<int> dataCounts;
  std::vector<int> dataDispls;
  for (size_t jt = 0; jt < comm_.size(); ++jt) {
    dataCounts.push_back(vars_.size()*obsSpace_.sizeOwn(jt));
  }
  dataDispls.push_back(0);
  for (size_t jt = 0; jt < comm_.size()-1; ++jt) {
    dataDispls.push_back(dataDispls[jt]+dataCounts[jt]);
  }

  // Local vector
  std::vector<double> randomPertOwn(obsSpace_.sizeOwn());

  // Scatter perturbation
  comm_.scatterv(randomPertGlb.begin(), randomPertGlb.end(), dataCounts, dataDispls,
    randomPertOwn.begin(), randomPertOwn.end(), 0);

  // Format data
  for (size_t jvar = 0; jvar < vars_.size(); ++jvar) {
    atlas::Field field = data_[vars_[jvar].name()];
    auto view = atlas::array::make_view<double, 2>(field);
    for (size_t jo = 0; jo < obsSpace_.sizeOwn(); ++jo) {
      view(jo, 0) = randomPertOwn[jo*vars_.size()+jvar];
    }
  }

  // Fill halo
  fillHalo();

  oops::Log::trace() << classname() << "::random done" << std::endl;
}

// -----------------------------------------------------------------------------

double ObsVector::dot_product_with(const ObsVector & other) const {
  oops::Log::trace() << classname() << "::dot_product_with starting" << std::endl;

  double zz = 0;
  for (size_t jvar = 0; jvar < vars_.size(); ++jvar) {
    const atlas::Field field = data_[vars_[jvar].name()];
    const auto view = atlas::array::make_view<double, 2>(field);
    const atlas::Field otherField = other.data_[vars_[jvar].name()];
    const auto otherView = atlas::array::make_view<double, 2>(otherField);
    for (size_t jo = 0; jo < obsSpace_.sizeOwn(); ++jo) {
      zz += view(jo, 0)*otherView(jo, 0);
    }
  }
  comm_.allReduceInPlace(zz, eckit::mpi::sum());

  oops::Log::trace() << classname() << "::dot_product_with done" << std::endl;
  return zz;
}

// -----------------------------------------------------------------------------

double ObsVector::rms() const {
  oops::Log::trace() << classname() << "::rms starting" << std::endl;

  double zz = dot_product_with(*this);
  if (obsSpace_.sizeGlb() > 0) {
    zz = std::sqrt(zz/static_cast<double>(obsSpace_.sizeGlb()));
  }

  oops::Log::trace() << classname() << "::rms done" << std::endl;
  return zz;
}

// -----------------------------------------------------------------------------

void ObsVector::mask(const ObsVector & mask) {
  oops::Log::trace() << classname() << "::mask starting" << std::endl;

  for (size_t jvar = 0; jvar < vars_.size(); ++jvar) {
    const atlas::Field maskField = mask.data_[vars_[jvar].name()];
    const auto maskView = atlas::array::make_view<double, 2>(maskField);
    atlas::Field field = data_[vars_[jvar].name()];
    auto view = atlas::array::make_view<double, 2>(field);
    for (size_t jo = 0; jo < obsSpace_.sizeLoc(); ++jo) {
      if (maskView(jo, 0) == missing_) view(jo, 0) = missing_;
    }
  }

  oops::Log::trace() << classname() << "::mask done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsVector::sqrt() {
  oops::Log::trace() << classname() << "::sqrt starting" << std::endl;

  util::sqrtFieldSet(data_);

  oops::Log::trace() << classname() << "::sqrt done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsVector::get(const size_t & jvar,
                    const size_t & ii,
                    double & value) const {
  const atlas::Field field = data_[vars_[jvar].name()];
  ASSERT(ii < field.shape(0));
  const auto view = atlas::array::make_view<double, 2>(field);
  value = view(ii, 0);
}

// -----------------------------------------------------------------------------

void ObsVector::set(const size_t & jvar,
                    const size_t & ii,
                    const double & value) {
  atlas::Field field = data_[vars_[jvar].name()];
  ASSERT(ii < field.shape(0));
  auto view = atlas::array::make_view<double, 2>(field);
  view(ii, 0) = value;
}

// -----------------------------------------------------------------------------

void ObsVector::maskAndSerialize(const ObsVector & mask,
                                 std::vector<double> & values) const {
  oops::Log::trace() << classname() << "::maskAndSerialize starting" << std::endl;

  // Check whether halo was setup correctly
  ASSERT(obsSpace_.sizeLoc() > 0);

  // Get size
  size_t obsSize = 0;
  for (size_t jvar = 0; jvar < vars_.size(); ++jvar) {
    const atlas::Field maskField = mask.data_[vars_[jvar].name()];
    const auto maskView = atlas::array::make_view<double, 2>(maskField);
    atlas::Field field = data_[vars_[jvar].name()];
    auto view = atlas::array::make_view<double, 2>(field);
    for (size_t jo = 0; jo < obsSpace_.sizeLoc(); ++jo) {
      if ((view(jo, 0) != missing_) && (maskView(jo, 0) != missing_)) {
        obsSize++;
      }
    }
  }

  // Pack data
  std::vector<double> newValues(obsSize);
  size_t ii = 0;
  for (size_t jvar = 0; jvar < vars_.size(); ++jvar) {
    const atlas::Field maskField = mask.data_[vars_[jvar].name()];
    const auto maskView = atlas::array::make_view<double, 2>(maskField);
    atlas::Field field = data_[vars_[jvar].name()];
    auto view = atlas::array::make_view<double, 2>(field);
    for (size_t jo = 0; jo < obsSpace_.sizeLoc(); ++jo) {
      if ((view(jo, 0) != missing_) && (maskView(jo, 0) != missing_)) {
        newValues[ii++] = view(jo, 0);
      }
    }
  }
  values.insert(values.end(), newValues.begin(), newValues.end());

  oops::Log::trace() << classname() << "::maskAndSerialize done" << std::endl;
}

// -----------------------------------------------------------------------------

std::string ObsVector::info(const std::string & prefix) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  std::stringstream ss;
  this->print(ss);
  std::string grep = "\n" + prefix;
  if (!grep.empty() && std::isalnum(grep.back())) grep += ": ";

  oops::Log::trace() << classname() << "::print done" << std::endl;
  return grep + ss.str();
}

// -----------------------------------------------------------------------------
/*
std::string ObsVector::info(const std::string & prefix,
                            const ObsDataQG<int> &) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  std::stringstream ss;
  this->print(ss);
  std::string grep = "\n" + prefix;
  if (!grep.empty() && std::isalnum(grep.back())) grep += ": ";

  oops::Log::trace() << classname() << "::print done" << std::endl;
  return grep + ss.str();
}
*/
// -----------------------------------------------------------------------------

void ObsVector::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  std::vector<double> zmin(vars_.size(), std::numeric_limits<double>::max());
  std::vector<double> zmax(vars_.size(), -std::numeric_limits<double>::max());
  std::vector<double> zrms(vars_.size(), 0.0);

  for (size_t jvar = 0; jvar < vars_.size(); ++jvar) {
    const atlas::Field field = data_[vars_[jvar].name()];
    const auto view = atlas::array::make_view<double, 2>(field);
    for (size_t jo = 0; jo < obsSpace_.sizeOwn(); ++jo) {
      if (view(jo, 0) < zmin[jvar]) zmin[jvar] = view(jo, 0);
      if (view(jo, 0) > zmax[jvar]) zmax[jvar] = view(jo, 0);
      zrms[jvar] += view(jo, 0)*view(jo, 0);
    }
  }

  comm_.allReduceInPlace(zmin.begin(), zmin.end(), eckit::mpi::min());
  comm_.allReduceInPlace(zmax.begin(), zmax.end(), eckit::mpi::max());
  if (obsSpace_.sizeGlb() > 0) {
    comm_.allReduceInPlace(zrms.begin(), zrms.end(), eckit::mpi::sum());
    const double norm = 1.0/static_cast<double>(obsSpace_.sizeGlb());
    for (auto & item : zrms) {
      item *= norm;
      item = std::sqrt(item);
    }
  }

  for (size_t jvar = 0; jvar < vars_.size(); ++jvar) {
    if (obsSpace_.sizeGlb() > 0) {
      os << vars_[jvar].name() << " nobs= " << obsSpace_. sizeGlb() << " Min="
         << zmin[jvar] << ", Max=" << zmax[jvar] << ", RMS=" << zrms[jvar] << std::endl;
    } else {
      os << vars_[jvar].name() << ": No observations." << std::endl;
    }
  }

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
