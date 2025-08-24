/*
 * (C) Copyright 2024 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <iomanip>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "atlas/functionspace.h"
#include "atlas/interpolation.h"

#include "eckit/config/Configuration.h"

#include "oops/generic/UnstructuredInterpolator.h"
#include "oops/util/Logger.h"
#include "oops/util/ObjectCounter.h"

#ifdef ENABLE_SABER
#include "saber/interpolation/AtlasInterpWrapper.h"
#endif

#include "vind/Geometry.h"

namespace atlas {
  class Field;
  class Grid;
  namespace grid {
    class Partitioner;
  }
}

namespace vind {

// -----------------------------------------------------------------------------

class Interpolation {
 public:
  static const std::string classname()
    {return "vind::Interpolation";}

  // Constructor/destructor
  Interpolation(const Geometry &,
                const std::string &,
                const atlas::Grid &,
                const atlas::FunctionSpace &,
                const std::string &);
  ~Interpolation() {}

  // Horizontal interpolation and adjoint
  void execute(const atlas::FieldSet &,
               atlas::FieldSet &) const;
  void executeAdjoint(atlas::FieldSet &,
                      const atlas::FieldSet &) const;

  // Vertical interpolation
  void insertVerticalInterpolation(const std::string &,
                                   const std::vector<std::array<size_t, 2>> &,
                                   const std::vector<std::array<double, 2>> &,
                                   const std::vector<size_t> &);
  void executeVertical(const atlas::FieldSet &,
                       atlas::FieldSet &) const;
  void executeVerticalAdjoint(atlas::FieldSet &,
                              const atlas::FieldSet &) const;

  // Accessors
  const std::string & srcUid() const
    {return srcUid_;}
  const std::string & tgtUid() const
    {return tgtUid_;}
  const atlas::FunctionSpace & tgtFspace() const
    {return tgtFspace_;}

 private:
  // Grids UID
  std::string srcUid_;
  std::string tgtUid_;

  // Destination function space
  atlas::FunctionSpace tgtFspace_;

#ifdef ENABLE_SABER
  // ATLAS interpolation wrapper from SABER
  std::shared_ptr<saber::interpolation::AtlasInterpWrapper> atlasInterpWrapper_;
#endif

  // Regional ATLAS interpolation
  std::shared_ptr<atlas::Interpolation> regionalInterp_;

  // OOPS unstructured interpolation
  std::shared_ptr<oops::UnstructuredInterpolator> unstructuredInterp_;

  // Vertical interpolations
  std::unordered_map<std::string, std::vector<std::array<size_t, 2>>> verStencil_;
  std::unordered_map<std::string, std::vector<std::array<double, 2>>> verWeights_;
  std::unordered_map<std::string, std::vector<size_t>> verStencilSize_;
};

}  // namespace vind
