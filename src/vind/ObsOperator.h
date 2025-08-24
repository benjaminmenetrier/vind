/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <map>
#include <memory>
#include <ostream>
#include <string>

#include "oops/interface/ObsOperatorBase.h"

#include "oops/util/ObjectCounter.h"

#include "vind/TraitsFwd.h"
#include "vind/Variables.h"

namespace eckit {
  class Configuration;
}

namespace oops {
  template <typename MODEL>
  class ObsAuxControlBase;
}

namespace util {
  class DateTime;
}

namespace vind {
  class GeoVaLs;
  class ObsAuxControl;
  class ObsSpace;
  class ObsVector;

// -----------------------------------------------------------------------------
/// ObsOperator class

class ObsOperator : public oops::ObsOperatorBase<Traits>,
                    private util::ObjectCounter<ObsOperator> {
  using ObsAuxControlPtrMap_ =
    typename std::map<std::string, std::unique_ptr<oops::ObsAuxControlBase<Traits>> >;

 public:
  static const std::string classname()
    {return "vind::ObsOperator";}

  ObsOperator(const ObsSpace &,
              const eckit::Configuration &);
  ~ObsOperator()
    {}

  void obsEquiv(const GeoVaLs &,
                ObsVector &,
                const ObsAuxControlPtrMap_ &) const;

  void obsBiasEquiv(const GeoVaLs &,
                    ObsVector &,
                    const ObsAuxControlPtrMap_ &) const
    {}

  std::shared_ptr<const Variables> variables() const
    {return inputs_;}

  const ObsSpace & table() const
    {return obsSpace_;}

 private:
  void print(std::ostream &) const;

  const ObsSpace & obsSpace_;
  std::shared_ptr<const Variables> inputs_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
