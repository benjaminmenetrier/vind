/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#pragma once

#include <string>

namespace vind {

// -----------------------------------------------------------------------------

class PythonInterpreter {
 public:
  static const std::string classname()
    {return "vind::PythonInterpreter";}

  // Constructor
  PythonInterpreter()
    {}

  // Destructor
  ~PythonInterpreter()
    {}

  // Initialize Python interpeter if needed
  void initialize() const;

  // Finalize Python interpeter if needed
  void finalize() const;
};

// -----------------------------------------------------------------------------

}  // namespace vind
