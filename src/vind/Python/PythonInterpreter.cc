/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "vind/Python/PythonInterpreter.h"

#include <pybind11/embed.h>

#include "oops/util/Logger.h"

namespace vind {

// -----------------------------------------------------------------------------

static size_t instancesCounter = 0;

// -----------------------------------------------------------------------------

void PythonInterpreter::initialize() const {
  oops::Log::trace() << classname() << "::initialize starting" << std::endl;

  if (instancesCounter == 0) {
    oops::Log::info() << "Initializing Python interpreter" << std::endl;
    pybind11::initialize_interpreter();
  }
  ++instancesCounter;

  oops::Log::trace() << classname() << "::initialize done" << std::endl;
}

// -----------------------------------------------------------------------------

void PythonInterpreter::finalize() const {
  oops::Log::trace() << classname() << "::finalize starting" << std::endl;

  if (instancesCounter == 1) {
    oops::Log::info() << "Finalizing Python interpreter" << std::endl;
    pybind11::finalize_interpreter();
  }
  --instancesCounter;

  oops::Log::trace() << classname() << "::finalize done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
