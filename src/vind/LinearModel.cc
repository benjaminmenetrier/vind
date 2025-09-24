/*
 * (C) Copyright 2025 Meteorologisk Institutt
 * 
 */

#include "vind/LinearModel.h"

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Increment.h"
#include "vind/State.h"

namespace vind {

// -----------------------------------------------------------------------------

LinearModel::LinearModel(const Geometry & geom,
                         const eckit::Configuration & config)
  : linearModel_() {
  oops::Log::trace() << classname() << "::LinearModel starting" << std::endl;

  linearModel_.reset(LinearModelFactory::create(geom, config));

  oops::Log::trace() << classname() << "::LinearModel done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModel::setTrajectory(const State & xx,
                                State & xlr,
                                const ModelAuxControl & xxAux) {
  oops::Log::trace() << classname() << "::setTrajectory starting" << std::endl;

  linearModel_->setTrajectory(xx, xlr, xxAux);

  oops::Log::trace() << classname() << "::setTrajectory done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModel::initializeTL(Increment & dx) const {
  oops::Log::trace() << classname() << "::initializeTL starting" << std::endl;

  linearModel_->initializeTL(dx);

  oops::Log::trace() << classname() << "::initializeTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModel::stepTL(Increment & dx,
                         const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepTL starting" << std::endl;

  linearModel_->stepTL(dx, dxAux);

  oops::Log::trace() << classname() << "::stepTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModel::finalizeTL(Increment & dx) const {
  oops::Log::trace() << classname() << "::finalizeTL starting" << std::endl;

  linearModel_->finalizeTL(dx);

  oops::Log::trace() << classname() << "::finalizeTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModel::initializeAD(Increment & dx) const {
  oops::Log::trace() << classname() << "::initializeAD starting" << std::endl;

  linearModel_->initializeAD(dx);

  oops::Log::trace() << classname() << "::initializeAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModel::stepAD(Increment & dx,
                         const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepAD starting" << std::endl;

  linearModel_->stepAD(dx, dxAux);

  oops::Log::trace() << classname() << "::stepAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModel::finalizeAD(Increment & dx) const {
  oops::Log::trace() << classname() << "::finalizeAD starting" << std::endl;

  linearModel_->finalizeAD(dx);

  oops::Log::trace() << classname() << "::finalizeAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModel::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

//  os << *linearModel_;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
