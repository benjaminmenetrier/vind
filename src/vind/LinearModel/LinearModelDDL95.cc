/*
 * (C) Copyright 2025 Meteorologisk Institutt
 * 
 */

#include "vind/LinearModel/LinearModelDDL95.h"

#include "oops/util/Logger.h"

#include "vind/Fields.h"
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

  // Internal time-step
  const double dti = static_cast<double>(timeResolution_.toSeconds())/36000.0;

  // Number of internal sub-time-steps
  nsub_ = static_cast<size_t>(dti/dti_sub_);

  // Sub-time-step
  dt_sub_ = util::Duration(static_cast<int64_t>(
    static_cast<double>(timeResolution_.toSeconds())/static_cast<double>(nsub_)));

  // Half sub-time-step
  dt_sub_half_ = util::Duration(static_cast<int64_t>(
    0.5*static_cast<double>(timeResolution_.toSeconds())/static_cast<double>(nsub_)));

  oops::Log::trace() << classname() << "::LinearModelDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::stepTL(Increment & dx,
                              const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepTL starting" << std::endl;

  // Get geometry
  const Geometry & geom(dx.fields().geometry());

  // Get function space
  const atlas::functionspace::StructuredColumns fs(geom.functionSpace());
 
  // Integrate over sub-time-steps with a RK2 scheme
  for (size_t jsub = 0; jsub < nsub_; ++jsub) {
    fs.haloExchange(dx.fields().fieldSet());
    Fields vt(dx.fields(), false);
    tendencyTL(dx.fields(), vt);
    auto vi(dx.fields());
    vi.axpy(0.5*dti_sub_, vt);
    vi.updateTime(dt_sub_half_);
    fs.haloExchange(vi.fieldSet());
    Fields vtt(vi, false);
    tendencyTL(vi, vtt);
    dx.fields().axpy(dti_sub_, vtt);
    dx.fields().updateTime(dt_sub_);
  }

  oops::Log::trace() << classname() << "::stepTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::stepAD(Increment & dx,
                              const ModelAuxIncrement & dxAux) const {
  oops::Log::trace() << classname() << "::stepAD starting" << std::endl;

  // Get geometry
  const Geometry & geom(dx.fields().geometry());

  // Get function space
  const atlas::functionspace::StructuredColumns fs(geom.functionSpace());
 
  // Integrate over sub-time-steps with a RK2 scheme
  for (size_t jsub = 0; jsub < nsub_; ++jsub) {
    fs.haloExchange(dx.fields().fieldSet());
    Fields vt(dx.fields(), false);
    tendencyAD(dx.fields(), vt);
    auto vi(dx.fields());
    vi.axpy(0.5*dti_sub_, vt);
    vi.updateTime(-dt_sub_half_);
    fs.haloExchange(vi.fieldSet());
    Fields vtt(vi, false);
    tendencyAD(vi, vtt);
    dx.fields().axpy(dti_sub_, vtt);
    dx.fields().updateTime(-dt_sub_);
  }

  oops::Log::trace() << classname() << "::stepAD done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "DDL95 linear model:" << std::endl;
  os << "- dt = " << timeResolution_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::tendencyTL(const Fields & fields,
                                  Fields & tendFields) const {
  oops::Log::trace() << classname() << "::tendencyTL starting" << std::endl;

  // Update all variables
  for (const auto & var : fields.variables()) { 
    // Get fields
    const auto field = fields.fieldSet()[var.name()];
    const auto trajField = traj_.at(fields.validTime()).fieldSet()[var.name()];
    auto tendField = tendFields.fieldSet()[var.name()];

    // Get function space
    atlas::functionspace::StructuredColumns fs(field.functionspace());
    const auto view_i = atlas::array::make_view<int, 1>(fs.index_i());
    const auto view_j = atlas::array::make_view<int, 1>(fs.index_j());
    ASSERT(fs.halo() >= 2);

    // Get ghost view
    const auto ghostView = atlas::array::make_view<int, 1>(fs.ghost());

    // Get views
    const auto view = atlas::array::make_view<double, 2>(field);
    const auto trajView = atlas::array::make_view<double, 2>(trajField);
    auto tendView = atlas::array::make_view<double, 2>(tendField);

    // Create tendency
    tendView.assign(0.0);

    for (int jnode = 0; jnode < field.shape(0); ++jnode) {
      if (ghostView(jnode) == 0) {
        // Get X/Y indices
        const size_t ix = view_i(jnode)-1;
        const size_t iy = view_j(jnode)-1;

        if ((ix > 1) && (ix < nx_-1) && (iy > 0) && (iy < ny_-1)) {
          // Inside computation zone

          // Retrieve array indices
          const int ixp1 = fs.index(ix+1, iy);
          const int ixm2 = fs.index(ix-2, iy);
          const int ixm1 = fs.index(ix-1, iy);
          const int iyp1 = fs.index(ix, iy+1);
          const int iym1 = fs.index(ix, iy-1);

          for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
            // Usual L95 in x direction
            tendView(jnode, jlevel) = (view(ixp1, jlevel)-view(ixm2, jlevel))*trajView(ixm1, jlevel)
              +(trajView(ixp1, jlevel)-trajView(ixm2, jlevel))*view(ixm1, jlevel)
              -view(jnode, jlevel);

            // Add diffusion to get larger scales
            tendView(jnode, jlevel) += nu_*(
              (view(ixp1, jlevel)-2.0*view(jnode, jlevel)+view(ixm1, jlevel))
              +(view(iyp1, jlevel)-2.0*view(jnode, jlevel)+view(iym1, jlevel)));
          }
        } else {
          // Outside computation zone
          for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
            tendView(jnode, jlevel) = -view(jnode, jlevel);
          }
        }
      } 
    }
  }

  oops::Log::trace() << classname() << "::tendencyTL done" << std::endl;
}

// -----------------------------------------------------------------------------

void LinearModelDDL95::tendencyAD(const Fields & fields,
                                  Fields & tendFields) const {
  oops::Log::trace() << classname() << "::tendencyAD starting" << std::endl;


  oops::Log::trace() << classname() << "::tendencyAD done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
