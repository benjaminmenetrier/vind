/*
 * (C) Copyright 2025 Meteorologisk Institutt
 * 
 */

#include "vind/Model/ModelDDL95.h"

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"
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
  x_.resize(nx_);
  for (size_t jx = 0; jx < nx_; ++jx) {
    x_[jx] = 2.0*M_PI*static_cast<double>(jx)/static_cast<double>(nx_);
  }
  y_.resize(ny_);
  for (size_t jy = 0; jy < ny_; ++jy) {
    y_[jy] = 2.0*M_PI*static_cast<double>(jy)/static_cast<double>(ny_);
  }

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

  oops::Log::trace() << classname() << "::ModelDDL95 done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelDDL95::step(State & xx,
                      const ModelAuxControl & xxAux) const {
  oops::Log::trace() << classname() << "::step starting" << std::endl;

  // Get geometry
  const Geometry & geom(xx.fields().geometry());

  // Get function space
  const atlas::functionspace::StructuredColumns fs(geom.functionSpace());
 
  // Integrate over sub-time-steps with a RK2 scheme
  for (size_t jsub = 0; jsub < nsub_; ++jsub) {
    fs.haloExchange(xx.fields().fieldSet());
    Fields vt(xx.fields(), false);
    tendency(xx.fields(), vt);
    auto vi(xx.fields());
    vi.axpy(0.5*dti_sub_, vt);
    vi.updateTime(dt_sub_half_);
    fs.haloExchange(vi.fieldSet());
    Fields vtt(vi, false);
    tendency(vi, vtt);
    xx.fields().axpy(dti_sub_, vtt);
    xx.fields().updateTime(dt_sub_);
  }

  oops::Log::trace() << classname() << "::step done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelDDL95::print(std::ostream & os) const {
  oops::Log::trace() << classname() << "::print starting" << std::endl;

  os << "DDL95 model:" << std::endl;
  os << "- dt = " << timeResolution_ << std::endl;

  oops::Log::trace() << classname() << "::print done" << std::endl;
}

// -----------------------------------------------------------------------------

void ModelDDL95::tendency(const Fields & fields,
                          Fields & tendFields) const {
  oops::Log::trace() << classname() << "::tendency starting" << std::endl;

  // Get valid time components
  int year, month, day, hour, minute, second;
  fields.validTime().toYYYYMMDDhhmmss(year, month, day, hour, minute, second);

  // Get number of seconds since 00:00:00
  const double t = static_cast<double>(hour*3600+minute*60+second);

  // Update all variables
  for (const auto & var : fields.variables()) { 
    // Get fields
    const auto field = fields.fieldSet()[var.name()];
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

          // Time-variable forcing
          const double FF = (1.0+0.4*std::sin(x_[ix]-omega_*t)*(1.0-std::cos(y_[iy])))*F_;

          // Retrieve array indices
          const int ixp1 = fs.index(ix+1, iy);
          const int ixm2 = fs.index(ix-2, iy);
          const int ixm1 = fs.index(ix-1, iy);
          const int iyp1 = fs.index(ix, iy+1);
          const int iym1 = fs.index(ix, iy-1);

          for (int jlevel = 0; jlevel < field.shape(1); ++jlevel) {
            // Usual L95 in x direction
            tendView(jnode, jlevel) = (view(ixp1, jlevel)-view(ixm2, jlevel))*view(ixm1, jlevel)
              -view(jnode, jlevel)+FF;

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

  oops::Log::trace() << classname() << "::tendency done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
