/*
 * (C) Copyright 2025 Meteorologisk Institutt
 * 
 */

#pragma once

#include <string>
#include <vector>

#include "atlas/field.h"

#include "eckit/mpi/Comm.h"

namespace vind {

// -----------------------------------------------------------------------------

void copyFieldSetWithoutFunctionSpace(const atlas::FieldSet &,
                                      atlas::FieldSet &);

// -----------------------------------------------------------------------------

double dotProductFieldSetsWithoutFunctionSpace(const atlas::FieldSet &,
                                               const atlas::FieldSet &,
                                               const std::vector<std::string> &,
                                               const eckit::mpi::Comm &);

// -----------------------------------------------------------------------------

}  // namespace vind
