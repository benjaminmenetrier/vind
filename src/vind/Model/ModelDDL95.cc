/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/Model/ModelDDL95.h"

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"
#include "vind/Increment.h"
#include "vind/State.h"

namespace vind {

// -----------------------------------------------------------------------------

static ModelMaker<ModelDDL95> makerDDL95_("DDL95");

// -----------------------------------------------------------------------------

ModelDDL95::ModelDDL95(const Geometry & geom,
                       const eckit::Configuration & config)
  : timeResolution_(config.getString("time step")) {
  oops::Log::trace() << classname() << "::ModelDDL95 starting" << std::endl;

  // Check geometry
  ASSERT((geom.gridType() == "regional") || (geom.gridType() == "structured") ||
    (geom.gridType() == "regular_lonlat") || (geom.gridType() == "zonal_band"));

  // Get dimensions
  const atlas::RegularGrid & grid = geom.grid();
  nx_ = grid.nx();
  ny_ = grid.ny();

  // Define x/y coordinates
  const atlas::functionspace::StructuredColumns fs(geom.functionSpace());
  lonLatField_ = fs.xy().clone();
  auto lonlatView = atlas::array::make_view<double, 2>(lonLatField_);
  const double deg2rad = M_PI/180.0;
  for (int jnode = 0; jnode < lonLatField_.shape(0); ++jnode) {
    lonlatView(jnode, 0) *= deg2rad;
    lonlatView(jnode, 1) *= deg2rad;
  }

  // Get mask boundaries
  if (grid.periodic()) {
    ixMin_ = 0;
    ixMax_ = nx_-1;
  } else {
    ixMin_ = 2;
    ixMax_ = nx_-2;
  }
  if (geom.gridType() == "regional") {
    iyMin_ = 0;
    iyMax_ = ny_-1;
  } else {
    iyMin_ = 1;
    iyMax_ = ny_-2;
  }

  // Internal time-step
  dti_ = static_cast<double>(timeResolution_.toSeconds())/36000.0;

  // Half time-step
  dt_half_ = util::Duration(static_cast<int64_t>(
    0.5*static_cast<double>(timeResolution_.toSeconds())));

  oops::Log::trace() << classname() << "::ModelDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelDDL95::step(State & xx,
                      const ModelAuxControl & xxAux) const {
  oops::Log::trace() << classname() << "::step starting" << std::endl;

  // First step
  Increment dxTen1(xx.geometry(), xx.variables(), xx.validTime());
  xx.zeroHalo();
  xx.fieldSet().haloExchange();
  tendency(xx, dxTen1);
  dxTen1 *= 0.5*dti_;
  State xxTmp(xx);
  xxTmp += dxTen1;
  xxTmp.updateTime(dt_half_);

  // Second step
  Increment dxTen2(xx.geometry(), xx.variables(), xx.validTime());
  xxTmp.zeroHalo();
  xxTmp.fieldSet().haloExchange();
  tendency(xxTmp, dxTen2);
  dxTen2 *= dti_;
  xx += dxTen2;
  xx.updateTime(timeResolution_);

  oops::Log::trace() << classname() << "::step done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelDDL95::tendency(const State & xx,
                          Increment & dxTen) const {
  oops::Log::trace() << classname() << "::tendency starting" << std::endl;

  // Assert number of variables
  ASSERT(xx.variables().size() == 1);
  const oops::Variable var = xx.variables()[0];

  // Get valid time components
  int year, month, day, hour, minute, second;
  xx.validTime().toYYYYMMDDhhmmss(year, month, day, hour, minute, second);

  // Get number of seconds since 00:00:00
  const double t = static_cast<double>(hour*3600+minute*60+second);

  // Get fields
  const auto field = xx.fieldSet()[var.name()];
  auto tendField = dxTen.fieldSet()[var.name()];

  // Get function space
  atlas::functionspace::StructuredColumns fs(field.functionspace());
  const auto view_i = atlas::array::make_view<int, 1>(fs.index_i());
  const auto view_j = atlas::array::make_view<int, 1>(fs.index_j());
  ASSERT(fs.halo() >= 2);

  // Get ghost view
  const auto ghostView = atlas::array::make_view<int, 1>(fs.ghost());

  // Get lon/lat view
  const auto lonlatView = atlas::array::make_view<double, 2>(lonLatField_);

  // Get views
  const auto view = atlas::array::make_view<double, 2>(field);
  auto viewTen = atlas::array::make_view<double, 2>(tendField);

  // Initialize tendency
  viewTen.assign(0.0);

  for (int jnode = 0; jnode < field.shape(0); ++jnode) {
    if (ghostView(jnode) == 0) {
      // Get X/Y indices
      const size_t ix = view_i(jnode)-1;
      const size_t iy = view_j(jnode)-1;

      if ((ix >= ixMin_) && (ix <= ixMax_) && (iy >= iyMin_) && (iy <= iyMax_)) {
        // Inside computation zone

        // Retrieve array indices
        const int ixp1 = fs.index(ix+1, iy);
        const int ixm2 = fs.index(ix-2, iy);
        const int ixm1 = fs.index(ix-1, iy);
        const int iyp1 = fs.index(ix, iy+1);
        const int iym1 = fs.index(ix, iy-1);

        // Time-dependent forcing
        const double FF = (1.0+0.4*std::sin(lonlatView(jnode, 0)-omega_*t)
          *std::cos(lonlatView(jnode, 1)))*F_;

        for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
          // Usual L95 in x direction
          viewTen(jnode, jlevel) = (view(ixp1, jlevel)-view(ixm2, jlevel))*view(ixm1, jlevel)
            -view(jnode, jlevel)+FF;

          // X-direction diffusion
          viewTen(jnode, jlevel) += nu_*(view(ixp1, jlevel)+view(ixm1, jlevel)
            -2.0*view(jnode, jlevel));

          // Y-direction diffusion
          if ((iy > iyMin_) && (iy < iyMax_)) {
           viewTen(jnode, jlevel) += nu_*(view(iyp1, jlevel)+view(iym1, jlevel)
             -2.0*view(jnode, jlevel));
          }
        }
      }
    }
  }

  oops::Log::trace() << classname() << "::tendency done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelDDL95::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "DDL95 model:" << std::endl;
  os << "- dt: " << timeResolution_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
