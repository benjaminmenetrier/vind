/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/ObsOperator.h"

#include <string>
#include <vector>

#include "eckit/config/Configuration.h"
#include "eckit/exception/Exceptions.h"

#include "oops/util/DateTime.h"
#include "oops/util/Logger.h"

#include "vind/GeoVaLs.h"
#include "vind/ObsAuxControl.h"
#include "vind/ObsVector.h"
#include "vind/TraitsFwd.h"
#include "vind/Variables.h"

namespace vind {

// -----------------------------------------------------------------------------

static oops::ObsOperatorMaker<Traits, ObsOperator> makerObsOperatorDefault_("default");

// -----------------------------------------------------------------------------

ObsOperator::ObsOperator(const ObsSpace & obsSpace,
                         const eckit::Configuration & conf)
  : obsSpace_(obsSpace), inputs_(new Variables(conf)) {
  oops::Log::trace() << classname() << "::obsEquiv" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsOperator::obsEquiv(const GeoVaLs & gv,
                           ObsVector & obsVector,
                           const ObsAuxControlPtrMap_ & bias) const {
  oops::Log::trace() << classname() << "::obsEquiv starting" << std::endl;

  for (size_t jvar = 0; jvar < gv.fieldSet().size(); ++jvar) {
    // Get GeoVaLs view
    const auto gvField = gv.fieldSet()[jvar];
    const auto gvView = atlas::array::make_view<double, 2>(gvField);

    // Get bias
    double bias_ = 0.0;
/*
    // TODO(Benjamin): bias correction
    using icst_ = typename ObsAuxControlPtrMap_::const_iterator;
    icst_ it = bias.find("ObsAuxControl");
    if (it != bias.end()) {
      const ObsAuxControl * pbias = dynamic_cast<const ObsAuxControl*>(it->second.get());
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

  oops::Log::trace() << classname() << "::obsEquiv done" << std::endl;
}

// -----------------------------------------------------------------------------

void ObsOperator::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "ObsOperator: vind";

  oops::Log::trace() << classname() << "::print end" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
