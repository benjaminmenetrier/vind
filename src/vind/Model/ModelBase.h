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

namespace util {
  class DateTime;
}

namespace vind {
  class Geometry;
  class ModelAuxControl;
  class State;

// -----------------------------------------------------------------------------
///  ModelBase class

class ModelBase : public util::Printable {
 public:
  // Constructors/destructor
  ModelBase() = default;
  virtual ~ModelBase() = default;

  // Prepare model integration
  virtual void initialize(State &) const = 0;

  // Model integration
  virtual void step(State &,
                    const ModelAuxControl &) const = 0;

  // Finish model integration
  virtual void finalize(State &) const = 0;

  // Utilities
  virtual const util::Duration & timeResolution() const = 0;

 private:
  void print(std::ostream &) const override = 0;
};

// -----------------------------------------------------------------------------
///  ModelFactory class

class ModelFactory {
 public:
  static ModelBase * create(const Geometry &,
                            const eckit::Configuration &);

  virtual ~ModelFactory() = default;

 protected:
  explicit ModelFactory(const std::string & name);

 private:
  virtual ModelBase * make(const Geometry &,
                           const eckit::Configuration &) = 0;

  static std::map < std::string, ModelFactory * > & getMakers() {
    static std::map < std::string, ModelFactory * > makers_;
    return makers_;
  }
};

// -----------------------------------------------------------------------------
///  ModelMaker class

template<class T>
class ModelMaker : public ModelFactory {
 public:
  explicit ModelMaker(const std::string & name) : ModelFactory(name) {}

  ModelBase * make(const Geometry & geom,
                   const eckit::Configuration & config) override {
    oops::Log::trace() << "ModelBase::make starting" << std::endl;
    return new T(geom, config);
  }
};

// -----------------------------------------------------------------------------

}  // namespace vind
