/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/LinearObsOperator.h"

#include <string>

#include "eckit/config/Configuration.h"
#include "eckit/exception/Exceptions.h"

#include "vind/GeoVaLs.h"
#include "vind/ObsAuxIncrement.h"
#include "vind/ObsSpace.h"
#include "vind/ObsVector.h"
#include "vind/Variables.h"

namespace vind {

// -----------------------------------------------------------------------------

static oops::LinearObsOpMaker<Traits, LinearObsOperator> makerLinearObsOpDefault_("default");

// -----------------------------------------------------------------------------

LinearObsOperator::LinearObsOperator(const ObsSpace &,
                                     const eckit::Configuration & config)
  : inputs_(new Variables(config)) {
  oops::Log::trace() << classname() << "::LinearObsOperator" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearObsOperator::obsEquivTL(const GeoVaLs & gv,
                                   ObsVector & obsVector,
                                   const ObsAuxIncrementPtrMap_ & bias) const {
  oops::Log::trace() << classname() << "::obsEquivTL starting" << std::endl;

  for (size_t jvar = 0; jvar < gv.fieldSet().size(); ++jvar) {
    // Get GeoVaLs view
    const auto gvField = gv.fieldSet()[jvar];
    const auto gvView = atlas::array::make_view<double, 2>(gvField);

    // Get bias
    double bias_ = 0.0;
/*
    // TODO(Benjamin): bias correction
    using icst_ = typename ObsAuxIncrementPtrMap_::const_iterator;
    icst_ it = bias.find("ObsAuxControl");
    if (it != bias.end()) {
      const ObsAuxIncrement * pbias = dynamic_cast<const ObsAuxIncrement*>(it->second.get());
      ASSERT(pbias != nullptr);
      bias_ = (*pbias).value();
    }
*/

    // Compute observation equivalent
    for (int jo = 0; jo < gvField.shape(0); ++jo) {
      const int ii = gv.obsIndex(jo);
      obsVector.set(jvar, ii, gvView(jo, 0)+bias_);
    }
  }

  // Fill halo
  obsVector.fillHalo();

  oops::Log::trace() << classname() << "::obsEquivTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearObsOperator::obsEquivAD(GeoVaLs & gv,
                                   const ObsVector & obsVector,
                                   ObsAuxIncrementPtrMap_ & bias) const {
  oops::Log::trace() << classname() << "::obsEquivAD starting" << std::endl;

  for (size_t jvar = 0; jvar < gv.fieldSet().size(); ++jvar) {
    // Get GeoVaLs view
    auto gvField = gv.fieldSet()[jvar];
    auto gvView = atlas::array::make_view<double, 2>(gvField);

    // Get bias
/*
    // TODO(Benjamin): bias correction
    using iter_ = typename ObsAuxIncrementPtrMap_::iterator;
    iter_ it = bias.find("ObsAuxControl");
    if (it != bias.end()) {
      ObsAuxIncrement * pbias = dynamic_cast<ObsAuxIncrement*>(it->second.get());
      ASSERT(pbias != nullptr);
      for (int jo = 0; jo < gvField.shape(0); ++jo) {
        const int ii = gv.obsIndex(jo);
        v.get(jvar, ii, bias);
      }
    }
*/

    // Compute observation equivalent
    for (int jo = 0; jo < gvField.shape(0); ++jo) {
      const int ii = gv.obsIndex(jo);
      obsVector.get(jvar, ii, gvView(jo, 0));
    }
  }

  oops::Log::trace() << classname() << "::obsEquivAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearObsOperator::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "LinearObsOperator: vind LinearObsOperator";

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
