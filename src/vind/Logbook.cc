/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#include "vind/Logbook.h"

namespace vind {

// -----------------------------------------------------------------------------

void Logbook::start() {
  oops::Log::trace() << classname() << "::start starting" << std::endl;

  getInstance().connectToLogbook();

  oops::Log::trace() << classname() << "::start done" << std::endl;
}

// -----------------------------------------------------------------------------

Logbook & Logbook::getInstance() {
  oops::Log::trace() << classname() << "::getInstance starting" << std::endl;

  static Logbook theLogbook;

  oops::Log::trace() << classname() << "::getInstance done" << std::endl;
  return theLogbook;
}

// -----------------------------------------------------------------------------

Logbook::Logbook()
  : conf_(new eckit::LocalConfiguration()) {
  oops::Log::trace() << classname() << "::Logbook" << std::endl;
}

// -----------------------------------------------------------------------------

void Logbook::update(const eckit::Configuration & conf) {
  oops::Log::trace() << classname() << "::update starting" << std::endl;

  conf_.reset(new eckit::LocalConfiguration(conf));

  oops::Log::trace() << classname() << "::update done" << std::endl;
}

// -----------------------------------------------------------------------------

void Logbook::connectToLogbook() {
  oops::Log::trace() << classname() << "::connectToLogbook" << std::endl;
  util::LogbookHelper::connectCallback([this]
                                       (const eckit::Configuration & conf)
                                       {return update(conf);});
}

// -----------------------------------------------------------------------------

void Logbook::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << *conf_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
