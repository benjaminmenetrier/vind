/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/LinearModel/LinearModelDDL95.h"

#include "oops/util/Logger.h"

#include "vind/Geometry.h"
#include "vind/Increment.h"

namespace vind {

// -----------------------------------------------------------------------------

static LinearModelMaker<LinearModelDDL95> makerDDL95_("DDL95");

// -----------------------------------------------------------------------------

LinearModelDDL95::LinearModelDDL95(const Geometry & geom,
                                   const eckit::Configuration & config)
  : timeResolution_(config.getString("time step")) {
  oops::Log::trace() << classname() << "::LinearModelDDL95 starting" << std::endl;

  // Check geometry
  ASSERT((geom.gridType() == "regional") || (geom.gridType() == "structured") ||
    (geom.gridType() == "regular_lonlat") || (geom.gridType() == "zonal_band"));

  // Get dimensions
  const atlas::RegularGrid & grid = geom.grid();
  nx_ = grid.nx();
  ny_ = grid.ny();

  // Get computation zone boundaries
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

  // Define model to compute the whole trajectory
  model_.reset(new ModelDDL95(geom, config));

  oops::Log::trace() << classname() << "::LinearModelDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::setTrajectory(const State & xx,
                                     State & xlr,
                                     const ModelAuxControl & xxAux) {
  oops::Log::trace() << classname() << "::setTrajectory starting" << std::endl;

  // Save trajectory
  traj_.insert({xlr.validTime(), xlr});

  oops::Log::trace() << classname() << "::setTrajectory done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::stepTL(Increment & dx,
                              const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepTL starting" << std::endl;

  // Get initial trajectory state
  State xxTraj1(traj_.at(dx.validTime()));
  xxTraj1.zeroHalo();
  xxTraj1.fieldSet().haloExchange();

  // Compute intermediate trajectory state
  Increment dxTrajTen(xxTraj1.geometry(), xxTraj1.variables(), xxTraj1.validTime());
  model_->tendency(xxTraj1, dxTrajTen);
  dxTrajTen *= 0.5*dti_;
  dxTrajTen.updateTime(dt_half_);
  State xxTraj2(xxTraj1);
  xxTraj2.updateTime(dt_half_);
  xxTraj2 += dxTrajTen;
  xxTraj2.zeroHalo();
  xxTraj2.fieldSet().haloExchange();

  // First step
  Increment dxTen1(dx, false);
  dx.zeroHalo();
  dx.fieldSet().haloExchange();
  tendencyTL(dx, xxTraj1, dxTen1);
  dxTen1 *= 0.5*dti_;
  Increment dxTmp(dx);
  dxTmp += dxTen1;
  dxTmp.updateTime(dt_half_);

  // Second step
  Increment dxTen2(dx, false);
  dxTmp.zeroHalo();
  dxTmp.fieldSet().haloExchange();
  tendencyTL(dxTmp, xxTraj2, dxTen2);
  dxTen2 *= dti_;
  dx += dxTen2;
  dx.updateTime(timeResolution_);

  oops::Log::trace() << classname() << "::stepTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::stepAD(Increment & dx,
                              const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepAD starting" << std::endl;

  // Get initial trajectory state
  State xxTraj1(traj_.at(dx.validTime()-timeResolution_));
  xxTraj1.zeroHalo();
  xxTraj1.fieldSet().haloExchange();

  // Compute intermediate trajectory state
  Increment dxTrajTen(xxTraj1.geometry(), xxTraj1.variables(), xxTraj1.validTime());
  model_->tendency(xxTraj1, dxTrajTen);
  dxTrajTen *= 0.5*dti_;
  dxTrajTen.updateTime(dt_half_);
  State xxTraj2(xxTraj1);
  xxTraj2.updateTime(dt_half_);
  xxTraj2 += dxTrajTen;
  xxTraj2.zeroHalo();
  xxTraj2.fieldSet().haloExchange();

  // Set increment halo to zero
  dx.zeroHalo();

  // Second step
  Increment dxTen2(dx);
  dxTen2 *= dti_;
  Increment dxTmp(dx, false);
  tendencyAD(dxTen2, xxTraj2, dxTmp);
  dxTmp.fieldSet().adjointHaloExchange();
  dxTmp.fieldSet().set_dirty();
  dx += dxTmp;
  dx.updateTime(-dt_half_);

  // First step
  dxTmp.updateTime(-dt_half_);
  Increment dxTen1(dxTmp);
  dxTen1 *= 0.5*dti_;
  tendencyAD(dxTen1, xxTraj1, dxTmp);
  dxTmp.fieldSet().adjointHaloExchange();
  dxTmp.fieldSet().set_dirty();
  dx += dxTmp;
  dx.updateTime(-dt_half_);

  oops::Log::trace() << classname() << "::stepAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "DDL95 linear model:" << std::endl;
  os << "- dt: " << timeResolution_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::tendencyTL(const Increment & dx,
                                  const State & xxTraj,
                                  Increment & dxTen) const {
  oops::Log::trace() << classname() << "::tendencyTL starting" << std::endl;

  // Assert number of variables
  ASSERT(dx.variables().size() == 1);
  const oops::Variable var = dx.variables()[0];

  // Get fields
  const auto field = dx.fieldSet()[var.name()];
  const auto fieldTraj = xxTraj.fieldSet()[var.name()];
  auto fieldTen = dxTen.fieldSet()[var.name()];

  // Get function space
  atlas::functionspace::StructuredColumns fs(field.functionspace());
  const auto view_i = atlas::array::make_view<int, 1>(fs.index_i());
  const auto view_j = atlas::array::make_view<int, 1>(fs.index_j());
  ASSERT(fs.halo() >= 2);

  // Get ghost view
  const auto ghostView = atlas::array::make_view<int, 1>(fs.ghost());

  // Get views
  const auto view = atlas::array::make_view<double, 2>(field);
  const auto viewTraj = atlas::array::make_view<double, 2>(fieldTraj);
  auto viewTen = atlas::array::make_view<double, 2>(fieldTen);

  // Initialization
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

        for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
          // Usual L95 in x direction
          viewTen(jnode, jlevel) = (view(ixp1, jlevel)-view(ixm2, jlevel))*viewTraj(ixm1, jlevel)
            +(viewTraj(ixp1, jlevel)-viewTraj(ixm2, jlevel))*view(ixm1, jlevel)
            -view(jnode, jlevel);

          // X-direction diffusion
          viewTen(jnode, jlevel) += nu_*(view(ixp1, jlevel)-2.0*view(jnode, jlevel)
            +view(ixm1, jlevel));

          // Y-direction diffusion
          if ((iy > iyMin_) && (iy < iyMax_)) {
            viewTen(jnode, jlevel) += nu_*(view(iyp1, jlevel)-2.0*view(jnode, jlevel)
              +view(iym1, jlevel));
          }
        }
      }
    }
  }

  fieldTen.set_dirty();

  oops::Log::trace() << classname() << "::tendencyTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::tendencyAD(const Increment & dxTen,
                                  const State & xxTraj,
                                  Increment & dx) const {
  oops::Log::trace() << classname() << "::tendencyAD starting" << std::endl;

  // Assert number of variables
  ASSERT(dx.variables().size() == 1);
  const oops::Variable var = dx.variables()[0];

  // Get fields
  auto field = dx.fieldSet()[var.name()];
  const auto fieldTraj = xxTraj.fieldSet()[var.name()];
  const auto fieldTen = dxTen.fieldSet()[var.name()];

  // Get function space
  atlas::functionspace::StructuredColumns fs(field.functionspace());
  const auto view_i = atlas::array::make_view<int, 1>(fs.index_i());
  const auto view_j = atlas::array::make_view<int, 1>(fs.index_j());
  ASSERT(fs.halo() >= 2);

  // Get ghost view
  const auto ghostView = atlas::array::make_view<int, 1>(fs.ghost());

  // Get views
  auto view = atlas::array::make_view<double, 2>(field);
  const auto viewTraj = atlas::array::make_view<double, 2>(fieldTraj);
  const auto viewTen = atlas::array::make_view<double, 2>(fieldTen);

  // Initialization
  view.assign(0.0);

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

        for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
          // Usual L95 in x direction
          view(ixp1, jlevel) += viewTen(jnode, jlevel)*viewTraj(ixm1, jlevel);
          view(ixm2, jlevel) -= viewTen(jnode, jlevel)*viewTraj(ixm1, jlevel);
          view(ixm1, jlevel) += viewTen(jnode, jlevel)
            *(viewTraj(ixp1, jlevel)-viewTraj(ixm2, jlevel));
          view(jnode, jlevel) -= viewTen(jnode, jlevel);

          // X-direction diffusion
          view(ixp1, jlevel) += nu_*viewTen(jnode, jlevel);
          view(jnode, jlevel) -= 2.0*nu_*viewTen(jnode, jlevel);
          view(ixm1, jlevel) += nu_*viewTen(jnode, jlevel);

          // Y-direction diffusion
          if ((iy > iyMin_) && (iy < iyMax_)) {
            view(iyp1, jlevel) += nu_*viewTen(jnode, jlevel);
            view(jnode, jlevel) -= 2.0*nu_*viewTen(jnode, jlevel);
            view(iym1, jlevel) += nu_*viewTen(jnode, jlevel);
          }
        }
      }
    }
  }

  field.set_dirty();

  oops::Log::trace() << classname() << "::tendencyAD done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
