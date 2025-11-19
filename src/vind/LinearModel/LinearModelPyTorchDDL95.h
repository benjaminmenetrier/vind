/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <pybind11/embed.h>
#include <torch/extension.h>
#include <torch/torch.h>

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
/// LinearModelTorchPyDDL95 class

class __attribute__((visibility("hidden"))) LinearModelTorchPyDDL95:
  public LinearModelBase,
  private util::ObjectCounter<LinearModelTorchPyDDL95> {
 public:
  static const std::string classname() {return "vind::LinearModelTorchPyDDL95";}

  // Constructor/destructor
  LinearModelTorchPyDDL95(const Geometry &,
                          const eckit::Configuration &);
  ~LinearModelTorchPyDDL95();

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
  torch::TensorOptions opts_;
  torch::Tensor lonTTensor_;
  torch::Tensor latTTensor_;
  torch::Tensor cMaskTTensor_;
  torch::Tensor yMaskTTensor_;
  std::map<util::DateTime, State> traj_;
};
// -----------------------------------------------------------------------------

}  // namespace vind
