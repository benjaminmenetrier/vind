/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <memory>
#include <string>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/memory/NonCopyable.h"

#include "util/LogbookHelper.h"

namespace vind {

// -----------------------------------------------------------------------------
/// Logbook class

class Logbook : public util::Printable,
                private eckit::NonCopyable {
 public:
  static const std::string classname()
    {return "vind::Logbook";}

/// OOPS interface

// Initialization, finalization
  static void start();
  static void stop()
    {}

// Destructor
  ~Logbook()
    {}

 private:
  static Logbook & getInstance();
  Logbook();

// Basic operators
  void update(const eckit::Configuration &);
  void connectToLogbook();

// Print
  void print(std::ostream &) const;

// Data
  std::unique_ptr<eckit::LocalConfiguration> conf_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
