/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <pybind11/embed.h>

#include <map>
#include <memory>
#include <ostream>
#include <string>

#include "eckit/mpi/Comm.h"

#include "oops/util/DateTime.h"
#include "oops/util/Duration.h"
#include "oops/util/ObjectCounter.h"

#include "vind/LinearModel/LinearModelBase.h"
#include "vind/Model/ModelBase.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class Increment;
  class ModelAuxControl;
  class ModelAuxIncrement;

// -----------------------------------------------------------------------------
///  LinearModelNumPy class

class __attribute__((visibility("hidden"))) LinearModelNumPy:
  public LinearModelBase,
  private util::ObjectCounter<LinearModelNumPy> {
 public:
  static const std::string classname() {return "vind::LinearModelNumPy";}

  // Constructor/destructor
  LinearModelNumPy(const Geometry &,
                   const eckit::Configuration &);
  ~LinearModelNumPy();

  // Set the linearization trajectory
  void setTrajectory(const State &,
                     State &,
                     const ModelAuxControl &) override;

  // Prepare TL model integration
  void initializeTL(Increment &) const override;

  // TL model integration
  void stepTL(Increment &,
              const ModelAuxIncrement &) const override;

  // Finish TL model integration
  void finalizeTL(Increment &) const override;

  // Prepare AD model integration
  void initializeAD(Increment &) const override;

  // AD model integration
  void stepAD(Increment &,
              const ModelAuxIncrement &) const override;

  // Finish AD model integration
  void finalizeAD(Increment &) const override;

  // Utilities
  const util::Duration & timeResolution() const
    {return timeResolution_;}
  const util::Duration & stepTrajectory() const
    {return timeResolution_;}

 private:
  void print(std::ostream &) const override;

  const util::Duration timeResolution_;
  const eckit::mpi::Comm & comm_;
  const std::string pythonModule_;
  std::string pythonDir_;
  std::unique_ptr<pybind11::dict> initData_;
  std::unique_ptr<pybind11::object> linearModel_;
  std::map<util::DateTime, State> traj_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
