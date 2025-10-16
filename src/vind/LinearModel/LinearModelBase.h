/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <map>
#include <memory>
#include <string>

#include "eckit/config/Configuration.h"

#include "oops/util/Duration.h"
#include "oops/util/Logger.h"
#include "oops/util/Printable.h"

namespace vind {
  class Geometry;
  class Increment;
  class ModelAuxControl;
  class ModelAuxIncrement;
  class State;

// -----------------------------------------------------------------------------
/// LinearModelBase class

class LinearModelBase : public util::Printable {
 public:
  static const std::string classname() {return "vind::LinearModelBase";}

  // Constructor/destructor
  LinearModelBase() = default;
  virtual ~LinearModelBase() = default;

  // Set the linearization trajectory
  virtual void setTrajectory(const State &,
                             State &,
                             const ModelAuxControl &) = 0;

  // Prepare TL model integration
  virtual void initializeTL(Increment &) const = 0;

  // TL model integration
  virtual void stepTL(Increment &,
                      const ModelAuxIncrement &) const = 0;

  // Finish TL model integration
  virtual void finalizeTL(Increment &) const = 0;

  // Prepare AD model integration
  virtual void initializeAD(Increment &) const = 0;

  // AD model integration
  virtual void stepAD(Increment &,
                      const ModelAuxIncrement &) const = 0;

  // Finish AD model integration
  virtual void finalizeAD(Increment &) const = 0;

  // Utilities
  virtual const util::Duration & timeResolution() const = 0;
  virtual const util::Duration & stepTrajectory() const = 0;

 private:
  void print(std::ostream &) const override = 0;
};

// -----------------------------------------------------------------------------
/// LinearModelFactory class

class LinearModelFactory {
 public:
  static LinearModelBase * create(const Geometry &,
                                  const eckit::Configuration &);

  virtual ~LinearModelFactory() = default;

 protected:
  explicit LinearModelFactory(const std::string & name);

 private:
  virtual LinearModelBase * make(const Geometry &,
                                 const eckit::Configuration &) = 0;

  static std::map < std::string, LinearModelFactory * > & getMakers() {
    static std::map < std::string, LinearModelFactory * > makers_;
    return makers_;
  }
};

// -----------------------------------------------------------------------------
/// LinearModelMaker class

template<class T>
class LinearModelMaker : public LinearModelFactory {
 public:
  explicit LinearModelMaker(const std::string & name) : LinearModelFactory(name) {}

  LinearModelBase * make(const Geometry & geom,
                         const eckit::Configuration & config) override {
    oops::Log::trace() << "LinearModelBase::make starting" << std::endl;
    return new T(geom, config);
  }
};

// -----------------------------------------------------------------------------

}  // namespace vind
