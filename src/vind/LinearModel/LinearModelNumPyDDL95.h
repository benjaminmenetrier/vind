/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <pybind11/embed.h>
#include <pybind11/numpy.h>

#include <map>
#include <memory>
#include <ostream>
#include <string>

#include "eckit/mpi/Comm.h"

#include "oops/util/DateTime.h"
#include "oops/util/Duration.h"
#include "oops/util/ObjectCounter.h"

#include "vind/LinearModel/LinearModelBase.h"
#include "vind/Python/PythonInterpreter.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class Geometry;
  class Increment;
  class ModelAuxControl;
  class ModelAuxIncrement;

// -----------------------------------------------------------------------------
/// LinearModelNumPyDDL95 class

class __attribute__((visibility("hidden"))) LinearModelNumPyDDL95:
  public LinearModelBase,
  private util::ObjectCounter<LinearModelNumPyDDL95> {
 public:
  static const std::string classname() {return "vind::LinearModelNumPyDDL95";}

  // Constructor/destructor
  LinearModelNumPyDDL95(const Geometry &,
                        const eckit::Configuration &);
  ~LinearModelNumPyDDL95();

  // Set the linearization trajectory
  void setTrajectory(const State &,
                     State &,
                     const ModelAuxControl &) override;

  // Prepare TL model integration
  void initializeTL(Increment &) const override
    {}

  // TL model integration
  void stepTL(Increment &,
              const ModelAuxIncrement &) const override;

  // Finish TL model integration
  void finalizeTL(Increment &) const override
    {}

  // Prepare AD model integration
  void initializeAD(Increment &) const override
    {}

  // AD model integration
  void stepAD(Increment &,
              const ModelAuxIncrement &) const override;

  // Finish AD model integration
  void finalizeAD(Increment &) const override
    {}

  // Utilities
  const util::Duration & timeResolution() const
    {return timeResolution_;}
  const util::Duration & stepTrajectory() const
    {return timeResolution_;}

 private:
  void print(std::ostream &) const override;

  const util::Duration timeResolution_;
  const eckit::mpi::Comm & comm_;
  const PythonInterpreter pythonInterpreter_;
  std::unique_ptr<pybind11::dict> params_;
  std::unique_ptr<pybind11::array_t<double>> lonNArray_;
  std::unique_ptr<pybind11::array_t<double>> latNArray_;
  std::map<util::DateTime, State> traj_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
