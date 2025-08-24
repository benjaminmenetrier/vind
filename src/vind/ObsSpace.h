/*
 * (C) Copyright 2023 Meteorologisk Institutt
 * 
 */

#pragma once

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "atlas/field.h"
#include "atlas/util/Point.h"

#include "eckit/mpi/Comm.h"

#include "oops/util/DateTime.h"
#include "oops/util/Logger.h"
#include "oops/util/ObjectCounter.h"
#include "oops/util/Printable.h"

#include "vind/Geometry.h"
#include "vind/VariablesSwitch.h"

namespace eckit {
  class Configuration;
}

namespace vind {
  class GeoVaLs;
  class ObsVector;

// -----------------------------------------------------------------------------
/// ObsSpace class

class ObsSpace : public util::Printable,
                 private util::ObjectCounter<ObsSpace> {
 public:
  static const std::string classname()
    {return "vind::ObsSpace";}

  enum Write : bool {
    Screened = true,
    Original = false
  };

  ObsSpace(const eckit::Configuration &,
           const Geometry &,
           const util::DateTime &,
           const util::DateTime &,
           const bool lscreened = false);
  ~ObsSpace();

  const eckit::mpi::Comm & getComm() const
    {return comm_;}

  void putdb(const atlas::FieldSet &) const;
  void getdb(atlas::FieldSet &) const;

  std::vector<atlas::Point3> locations(const util::DateTime &,
                                       const util::DateTime &) const;
  std::vector<size_t> timeSelect(const util::DateTime &,
                                 const util::DateTime &) const;
  void generateDistribution(const eckit::Configuration &);
  void printJo(const ObsVector &,
               const ObsVector &);
  void screenObservations(const ObsVector &,
                          const GeoVaLs &) const;
  void saveObservations() const
    {}

  const size_t & sizeGlbAll() const
    {return nobsGlbAll_;}
  const size_t & sizeGlb() const
    {return nobsGlb_;}
  const size_t & sizeOwn() const
    {return nobsOwn_;}
  const size_t & sizeOwn(const size_t & it) const
    {return nobsOwnVec_[it];}
  const size_t & sizeLoc() const
    {return nobsLoc_;}
  const std::vector<int> & order() const
    {return order_;}
  const varns::Variables & vars() const
    {return vars_;}
  std::vector<atlas::Point3> & locations() const
    {return locs_;}
  void fillHalo(atlas::FieldSet &) const;
  const int64_t & getSeed() const
    {return seed_;}
  const std::vector<int> & maskSum() const
    {return maskSum_;}

 private:
  void print(std::ostream &) const;
  void read(const std::string &);
  void write(const std::string &,
             const bool &) const;
  void setupHalo() const;
  void checkValidity(const std::vector<float> &,
                     const std::vector<float> &);
  void splitObservations(const std::vector<float> &,
                         const std::vector<float> &);

  const util::DateTime winbgn_;
  const util::DateTime winend_;
  const bool lscreened_;
  const eckit::mpi::Comm & comm_;
  const std::shared_ptr<const Geometry> geom_;
  mutable std::vector<util::DateTime> times_;
  mutable std::vector<atlas::Point3> locs_;
  mutable std::vector<atlas::FieldSet> data_;
  mutable std::vector<util::DateTime> screenedTimes_;
  mutable std::vector<atlas::Point3> screendLocs_;
  mutable std::vector<atlas::FieldSet> screenedData_;
  std::string nameIn_;
  std::string nameOut_;
  size_t nobsGlbAll_;
  size_t nobsGlb_;
  size_t nobsOwn_;
  const varns::Variables vars_;
  std::vector<size_t> nobsOwnVec_;
  std::vector<int> order_;
  std::vector<int> mask_;
  std::vector<int> maskSum_;
  std::vector<int> partition_;
  eckit::LocalConfiguration distribution_;
  mutable size_t nobsLoc_;
  mutable std::vector<int> sendBufIndex_;
  mutable size_t nSend_;
  mutable size_t nRecv_;
  mutable std::vector<int> dataSendCounts_;
  mutable std::vector<int> dataRecvCounts_;
  mutable std::vector<int> dataSendDispls_;
  mutable std::vector<int> dataRecvDispls_;
  int64_t seed_;
};

// -----------------------------------------------------------------------------

}  // namespace vind
