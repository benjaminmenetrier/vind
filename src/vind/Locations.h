/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "atlas/grid.h"
#include "atlas/util/Point.h"

#include "oops/util/DateTime.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vind/ObsSpace.h"

namespace vind {

// -----------------------------------------------------------------------------
/// Locations class

class Locations : public util::Printable,
                  private util::ObjectCounter<Locations> {
 public:
  static const std::string classname()
    {return "vind::Locations";}

/// OOPS interface

// Constructor/destructor
  Locations(const ObsSpace &,
            const util::DateTime &,
            const util::DateTime &);
  ~Locations()
    {}

/// Local
  const ObsSpace & obsSpace() const
    {return obsSpace_;}
  int size() const
    {return locs_.size();}
  int size(const size_t & jt) const
    {return nobsOwnVec_[jt];}
  const atlas::Point3 & operator[](const int & ii) const
    {return locs_[ii];}
  const atlas::Grid & grid() const
    {return grid_;}

 private:
  void print(std::ostream & os) const;

  const ObsSpace & obsSpace_;
  std::vector<atlas::Point3> locs_;
  std::vector<int> nobsOwnVec_;
  atlas::Grid grid_;
};

}  // namespace vind
