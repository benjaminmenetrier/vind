/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "vind/FieldsIO/FieldsIOBSC.h"

#include <netcdf.h>
#include <stdint.h>

#include <cmath>
#include <memory>
#include <set>
#include <vector>

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"

#define ERR(e, msg) {std::string s(nc_strerror(e)); \
  throw eckit::Exception(s + " : " + msg, Here());}

namespace vind {

// -----------------------------------------------------------------------------

static FieldsIOMaker<FieldsIOBSC> makerBSC_("bsc");

// -----------------------------------------------------------------------------

static std::vector<std::string> existingFiles_;
static eckit::LocalConfiguration attributes_;

// -----------------------------------------------------------------------------

void FieldsIOBSC::read(const oops::Variables & vars,
                       const eckit::Configuration & conf,
                       Fields & fields) const {
  oops::Log::trace() << classname() << "::read starting" << std::endl;

  // Get geometry
  const Geometry & geom(fields.geometry());

  // Get function space
  const atlas::functionspace::StructuredColumns fs(geom.functionSpace());

  // Clear local fieldset
  fields.fieldSet().clear();

  // Create local fieldset
  for (const auto & var : vars) {
    atlas::Field field = fs.createField<double>(
      atlas::option::name(var.name()) | atlas::option::levels(var.getLevels()));
    fields.fieldSet().add(field);
  }

  // Initialize local fieldset
  for (auto & field : fields.fieldSet()) {
    auto view = atlas::array::make_view<double, 2>(field);
    view.assign(0.0);
  }

  // Global data
  atlas::FieldSet globalData;
  for (const auto & var : vars) {
    atlas::Field field = fs.createField<double>(
      atlas::option::name(var.name()) | atlas::option::levels(var.getLevels())
      | atlas::option::global());
    globalData.add(field);
  }

  // Set State or Increment flag
  const bool isState = conf.getBool("is state");

  // Get filepath
  const std::string ncFilePath = conf.getString("filepath");

  // Get file initial time
  const util::DateTime initialTime(geom.io().getString("initial date"));

  // Get file final time
  const util::DateTime finalTime(geom.io().getString("final date"));

  // Get optional time step
  const double timeStep = geom.io().getDouble("time step", 3600.0);

  // Get total number of hours
  ASSERT(finalTime >= initialTime);
  const size_t timeMax = (finalTime-initialTime).toSeconds()/timeStep+1;

  // Get read time
  const util::DateTime validTime(conf.getString("date"));

  // Difference in hours
  ASSERT(validTime >= initialTime);
  ASSERT(validTime <= finalTime);
  const size_t time = (validTime-initialTime).toSeconds()/timeStep;

  // NetCDF IDs
  int ncid, retval, time_id, var_id[vars.size()];

  if (geom.getComm().rank() == 0) {
    // Get grid
    const atlas::StructuredGrid grid = fs.grid();

    // Get sizes
    const size_t nx = grid.nxmax();
    const size_t ny = grid.ny();

    oops::Log::info() << "Info     : Reading file: " << ncFilePath << std::endl;

    // Open NetCDF file
    if ((retval = nc_open(ncFilePath.c_str(), NC_NOWRITE, &ncid))) ERR(retval, ncFilePath);

    // Check that initialTime is consistent with "time" metadata in file
    if ((retval = nc_inq_varid(ncid, "time", &time_id))) ERR(retval, "time");
    const std::string time_units_key = "units";
    size_t attlen;
    if ((retval = nc_inq_attlen(ncid, time_id, time_units_key.c_str(), &attlen)))
      ERR(retval, "time");
    char *time_units_char = reinterpret_cast<char*>(malloc(attlen+1));
    if ((retval = nc_get_att_text(ncid, time_id, time_units_key.c_str(), time_units_char)))
      ERR(retval, "time");
    const std::string time_units_value(time_units_char);
    const std::string initialTimeFromFileStr = time_units_value.substr(12, 10) + "T"
      + time_units_value.substr(23, 5) + ":00Z";
    const util::DateTime initialTimeFromFile(initialTimeFromFileStr);
    ASSERT(initialTime == initialTimeFromFile);

    // Check number of times
    if ((retval = nc_inq_dimid(ncid, "time", &time_id))) ERR(retval, "time");
    size_t timeMaxFromFile;
    if ((retval = nc_inq_dimlen(ncid, time_id, &timeMaxFromFile))) ERR(retval, "time");
    ASSERT(timeMax == timeMaxFromFile);

    for (size_t jvar = 0; jvar < vars.size(); ++jvar) {
      // Get variables ID
      if ((retval = nc_inq_varid(ncid, vars[jvar].name().c_str(), &var_id[jvar])))
        ERR(retval, vars[jvar].name());

      // Read and save all NetCDF for this variable, if necessary
      if (!attributes_.has(geom.grid().uid() + "." + vars[jvar].name())) {
        // Get variable ID
        int varid = var_id[jvar];

        // Get number of attributes
        int natts = 0;
        if ((retval = nc_inq_varnatts(ncid, varid, &natts))) ERR(retval, "inq_varnatts");

        // Create attributes list
        std::vector<eckit::LocalConfiguration> varAttrs(natts);

        // Loop over attributes
        for (int att_i = 0; att_i < natts; ++att_i) {
          // Get attribute name
          char attName[NC_MAX_NAME + 1];
          if ((retval = nc_inq_attname(ncid, varid, att_i, attName))) ERR(retval, "inq_attname");
          varAttrs[att_i].set("name", std::string(attName));

          // Get attributes type
          nc_type attType;
          size_t attLen = 0;
          if ((retval = nc_inq_att(ncid, varid, attName, &attType, &attLen)))
            ERR(retval, "inq_att");
          varAttrs[att_i].set("type", static_cast<int>(attType));

          // Get attributes tokens
          std::vector<std::string> tokens;
          if (attType == NC_CHAR) {
            std::vector<char> buf(attLen+1, '\0');
            if ((retval = nc_get_att_text(ncid, varid, attName, buf.data())))
              ERR(retval, "get_att_text");
            tokens.push_back(std::string(buf.data()));
          } else if (attType == NC_INT) {
            std::vector<int> buf(attLen);
            if ((retval = nc_get_att_int(ncid, varid, attName, buf.data())))
              ERR(retval, "get_att_int");
            for (size_t k=0; k < attLen; ++k) tokens.push_back(std::to_string(buf[k]));


          } else if (attType == NC_FLOAT) {
            std::vector<float> buf(attLen);
            if ((retval = nc_get_att_float(ncid, varid, attName, buf.data())))
              ERR(retval, "get_att_float");
            for (size_t k=0; k < attLen; ++k) tokens.push_back(std::to_string(buf[k]));
          } else if (attType == NC_DOUBLE) {
            std::vector<double> buf(attLen);
            if ((retval = nc_get_att_double(ncid, varid, attName, buf.data())))
              ERR(retval, "get_att_double");
            for (size_t k=0; k < attLen; ++k) tokens.push_back(std::to_string(buf[k]));
          } else if (attType == NC_SHORT) {
            std::vector<int16_t> buf(attLen);
            if ((retval = nc_get_att_short(ncid, varid, attName, buf.data())))
              ERR(retval, "get_att_short");
            for (size_t k=0; k < attLen; ++k) tokens.push_back(std::to_string(buf[k]));
          } else {
            std::vector<char> buf(attLen+1, '\0');
            if ((retval = nc_get_att_text(ncid, varid, attName, buf.data())))
              ERR(retval, "get_att_text_fallback");
            tokens.push_back(std::string(buf.data()));
          }
          varAttrs[att_i].set("tokens", tokens);
        }

        // Save attributes
        if (!attributes_.has(geom.grid().uid())) {
          // Create grid attributes and insert variable attributes
          eckit::LocalConfiguration gridAttrs;
          gridAttrs.set(vars[jvar].name(), varAttrs);
          attributes_.set(geom.grid().uid(), gridAttrs);
        } else {
          // Insert variable attributes
          attributes_.set(geom.grid().uid() + "." + vars[jvar].name(), varAttrs);
        }
      }

      // Get variable view
      auto varView = atlas::array::make_view<double, 2>(globalData[vars[jvar].name()]);

      // Get transformation parameters (for states only)
      double scaleFactor = 1.0;
      bool logTransf = false;
      double addConst = 0.0;
      if (isState) {
        for (const auto & item : geom.alias()) {
          if (item.getString("in file") == vars[jvar].name()) {
            scaleFactor = item.getDouble("scaling factor", 1.0);
            logTransf = item.getBool("log transform", false);
            if (logTransf) {
              addConst = item.getDouble("additive constant", 0.0);
            }
          }
        }
      }

      if (vars[jvar].getLevels() == 1) {
        // Read single level
        std::vector<double> zvar(nx*ny);
        const std::vector<size_t> startp({time, 0, 0});
        const std::vector<size_t> countp({1, ny, nx});
        if ((retval = nc_get_vars_double(ncid, var_id[jvar], startp.data(), countp.data(), NULL,
          zvar.data()))) ERR(retval, vars[jvar].name());

        // Deserialize data to view
        for (size_t j = 0; j < ny; ++j) {
          const atlas::idx_t jj = ny-1-j;
          for (atlas::idx_t i = 0; i < grid.nx(jj); ++i) {
            atlas::gidx_t gidx = grid.index(i, jj);
            if (logTransf) {
              varView(gidx, 0) = log10((zvar[jj*nx+i] * scaleFactor) + addConst);
            } else {
              varView(gidx, 0) = zvar[jj*nx+i] * scaleFactor;
            }
          }
        }
      } else {
        std::string var_in_code;
        for (const auto & item : geom.alias()) {
          if (item.getString("in file") == vars[jvar].name()) {
            var_in_code = item.getString("in code");
          }
        }
        size_t loopMax = geom.vertCoordAvg(var_in_code).size();
        for (size_t k = 0; k < loopMax; ++k) {
          // Read level
          std::vector<double> zvar(nx*ny);
          const std::vector<size_t> countp({1, 1, ny, nx});
          const std::vector<size_t>startp({time,
          static_cast<size_t>(geom.vertCoordAvg(var_in_code)[k]), 0, 0});
          if ((retval = nc_get_vars_double(ncid, var_id[jvar], startp.data(), countp.data(),
                                NULL, zvar.data()))) ERR(retval, vars[jvar].name());

          // Deserialize data to view
          for (size_t j = 0; j < ny; ++j) {
            const atlas::idx_t jj = ny-1-j;
            for (atlas::idx_t i = 0; i < grid.nx(jj); ++i) {
              atlas::gidx_t gidx = grid.index(i, jj);
              if (logTransf) {
                varView(gidx, k) = log10((zvar[jj*nx+i] * scaleFactor) + addConst);
              } else {
                varView(gidx, k) = zvar[jj*nx+i] * scaleFactor;
              }
            }
          }
        }
      }
    }

    // Close file
    if ((retval = nc_close(ncid))) ERR(retval, ncFilePath);
  }

  // Scatter data from main processor
  fs.scatter(globalData, fields.fieldSet());

  // Mark dirty to be safe
  fields.fieldSet().set_dirty();

  oops::Log::trace() << classname() << "::read done" << std::endl;
}

// -----------------------------------------------------------------------------

void FieldsIOBSC::write(const eckit::Configuration & conf,
                        const Fields & fields) const {
  oops::Log::trace() << classname() << "::write starting" << std::endl;

  // Get geometry
  const Geometry & geom(fields.geometry());

  // Get function space
  const atlas::functionspace::StructuredColumns fs(geom.functionSpace());

  // Define variables vector from fields.fieldSet()
  const std::vector<std::string> vars = fields.fieldSet().field_names();

  // Get filepath
  const std::string ncFilePath = conf.getString("filepath");

  // Set State or Increment flag
  const bool isState = conf.getBool("is state");

  // Check if domain is global
  const bool isGlobal = geom.grid().domain().type() == "global";

  // Check if this file already exists
  const bool existingFile = std::find(existingFiles_.begin(), existingFiles_.end(), ncFilePath)
    != existingFiles_.end();
  if (!existingFile) {
    existingFiles_.push_back(ncFilePath);
  }

  // Get total number of levels (from geometry section)
  const size_t lmMax = geom.io().getUnsigned("total number of levels");

  // Get write time
  const util::DateTime validTime(conf.getString("date"));

  // Get timeseries mode
  const bool singleDate = conf.getBool("single date", false);

  // Get optional time step
  const double timeStep = geom.io().getDouble("time step", 3600.0);

  // Get file initial and final time
  util::DateTime initialTime;
  util::DateTime finalTime;
  size_t timeOffset;

  if (!singleDate) {
    // Get file initial time
    initialTime = util::DateTime(geom.io().getString("initial date"));

    // Get file final time
    finalTime = util::DateTime(geom.io().getString("final date"));

    // Reference for time coordinate
    timeOffset = 0;
  } else {
    // Set initial time
    initialTime = validTime;

    // Set final time
    finalTime = validTime;

    // Reference for time coordinate
    timeOffset =
      (initialTime - util::DateTime(geom.io().getString("initial date"))).toSeconds()
      /timeStep;
  }

  // Get total number of hours
  ASSERT(finalTime >= initialTime);
  const size_t timeMax = (finalTime-initialTime).toSeconds()/timeStep+1;

  // Difference in hours
  ASSERT(validTime >= initialTime);
  ASSERT(validTime <= finalTime);
  const size_t time = (validTime-initialTime).toSeconds()/timeStep;

  // NetCDF IDs
  int retval, ncid, rlon_id, rlat_id, lm_id, lmp_id, time_id,
    dRlon_id[1], dRlat_id[1], dLm_id[1], dLmp_id[1], dTime_id[1],
    d2D_id[2], d3D_id[3], d4D_id[4], d4Dp_id[4],
    vRlon_id, vRlat_id, vLm_id, vLmp_id, vrp_id, vTime_id,
    lon_id, lat_id, var_id[vars.size()];

  // Prepare local coordinates and data
  atlas::FieldSet localData;
  if (!existingFile && !isGlobal) {
    atlas::Field lonLocal = fs.createField<double>(atlas::option::name("lon"));
    localData.add(lonLocal);
    atlas::Field latLocal = fs.createField<double>(atlas::option::name("lat"));
    localData.add(latLocal);
    const auto lonlatView = atlas::array::make_view<double, 2>(fs.lonlat());
    auto lonViewLocal = atlas::array::make_view<double, 1>(localData["lon"]);
    auto latViewLocal = atlas::array::make_view<double, 1>(localData["lat"]);
    for (atlas::idx_t jnode = 0; jnode < fs.lonlat().shape(0); ++jnode) {
       lonViewLocal(jnode) = lonlatView(jnode, 0);
       latViewLocal(jnode) = lonlatView(jnode, 1);
    }
  }
  for (size_t jvar = 0; jvar < vars.size(); ++jvar) {
    localData.add(fields.fieldSet()[vars[jvar]]);
  }
  // Check if there are fields on secondary vertical coordinates.
  std::string var_in_code;
  std::vector<size_t> lev_vect;
  for (size_t jvar = 0; jvar < vars.size(); ++jvar) {
    for (const auto & item : geom.alias()) {
      if (item.getString("in file") == vars[jvar]) {
        var_in_code = item.getString("in code");
      }
    }
    if (geom.levels(var_in_code) > 1) {
      lev_vect.push_back(geom.levels(var_in_code));
    }
  }
  std::set<size_t> lev_set(lev_vect.begin(), lev_vect.end());
  bool has_int_press = lev_set.size() == 2;
  if (has_int_press && *next(lev_set.begin(), 1) != *lev_set.begin() + 1) {
    throw eckit::UserError("Inconsistent definition of vertical levels", Here());
  }

  // Prepare global coordinates and data
  atlas::FieldSet globalData;
  if (!existingFile && !isGlobal) {
    atlas::Field lonGlobal = fs.createField<double>(
      atlas::option::name("lon") | atlas::option::global());
    globalData.add(lonGlobal);
    atlas::Field latGlobal = fs.createField<double>(
      atlas::option::name("lat") | atlas::option::global());
    globalData.add(latGlobal);
  }
  for (size_t jvar = 0; jvar < vars.size(); ++jvar) {
    atlas::Field globalField = fs.createField<double>(atlas::option::name(vars[jvar]) |
      atlas::option::levels(fields.fieldSet()[vars[jvar]].shape(1)) | atlas::option::global());
    globalData.add(globalField);
  }

  // Gather coordinates and data on main processor
  fs.gather(localData, globalData);

  if (geom.getComm().rank() == 0) {
    if (existingFile) {
      oops::Log::info() << "Info     : Updating file: " << ncFilePath << std::endl;
    } else {
      oops::Log::info() << "Info     : Writing file: " << ncFilePath << std::endl;
    }

    // Get grid
    const atlas::StructuredGrid grid = fs.grid();

    // Get sizes
    const size_t nx = grid.nxmax();
    size_t nx_out = nx;
    if (isGlobal) nx_out = nx+1;
    const size_t ny = grid.ny();

    // Definition mode

    if (existingFile) {
      // Open NetCDF file
      if ((retval = nc_open(ncFilePath.c_str(),
        NC_64BIT_OFFSET | NC_WRITE, &ncid))) ERR(retval, ncFilePath);

      // Switch to definition mode
      if ((retval = nc_redef(ncid))) ERR(retval, ncFilePath);
    } else {
      // Create NetCDF file
      if ((retval = nc_create(ncFilePath.c_str(),
        NC_64BIT_OFFSET | NC_CLOBBER, &ncid))) ERR(retval, ncFilePath);
    }

    if (existingFile) {
      // Get dimension
      if (isGlobal) {
        if ((retval = nc_inq_dimid(ncid, "lon", &rlon_id))) ERR(retval, "lon");
        if ((retval = nc_inq_dimid(ncid, "lat", &rlat_id))) ERR(retval, "lat");
      } else {
        if ((retval = nc_inq_dimid(ncid, "rlon", &rlon_id))) ERR(retval, "rlon");
        if ((retval = nc_inq_dimid(ncid, "rlat", &rlat_id))) ERR(retval, "rlat");
      }
      if ((retval = nc_inq_dimid(ncid, "lm", &lm_id))) ERR(retval, "lm");
      if (has_int_press) {
        if ((retval = nc_inq_dimid(ncid, "lmp", &lmp_id))) ERR(retval, "lmp");
      }
      if ((retval = nc_inq_dimid(ncid, "time", &time_id))) ERR(retval, "time");
    } else {
      // Create dimensions
      if (isGlobal) {
        if ((retval = nc_def_dim(ncid, "lon", nx_out, &rlon_id))) ERR(retval, "lon");
        if ((retval = nc_def_dim(ncid, "lat", ny, &rlat_id))) ERR(retval, "lat");
      } else {
        if ((retval = nc_def_dim(ncid, "rlon", nx_out, &rlon_id))) ERR(retval, "rlon");
        if ((retval = nc_def_dim(ncid, "rlat", ny, &rlat_id))) ERR(retval, "rlat");
      }
      if ((retval = nc_def_dim(ncid, "lm", lmMax, &lm_id))) ERR(retval, "lm");
      if (has_int_press) {
        if ((retval = nc_def_dim(ncid, "lmp", lmMax+1, &lmp_id))) ERR(retval, "lmp");
      }
      if ((retval = nc_def_dim(ncid, "time", NC_UNLIMITED, &time_id))) ERR(retval, "time");
    }

    // Dimensions arrays
    dRlon_id[0] = rlon_id;
    dRlat_id[0] = rlat_id;
    dLm_id[0] = lm_id;
    if (has_int_press) {
      dLmp_id[0] = lmp_id;
    }
    dTime_id[0] = time_id;
    d2D_id[0] = rlat_id;
    d2D_id[1] = rlon_id;
    d3D_id[0] = time_id;
    d3D_id[1] = rlat_id;
    d3D_id[2] = rlon_id;
    d4D_id[0] = time_id;
    d4D_id[1] = lm_id;
    d4D_id[2] = rlat_id;
    d4D_id[3] = rlon_id;
    if (has_int_press) {
      d4Dp_id[0] = time_id;
      d4Dp_id[1] = lmp_id;
      d4Dp_id[2] = rlat_id;
      d4Dp_id[3] = rlon_id;
    }

    // Attributes storage
    float float_att;
    char str_att[128];

    if (!existingFile) {
      // Define coordinates
      if (isGlobal) {
        // Lon
        if ((retval = nc_def_var(ncid, "lon", NC_FLOAT, 1, dRlon_id, &vRlon_id)))
          ERR(retval, "lon");
        strcpy(str_att, "longitude");
        if ((retval = nc_put_att_text(ncid, vRlon_id, "long_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lon long_name");
        strcpy(str_att, "degrees");
        if ((retval = nc_put_att_text(ncid, vRlon_id, "units", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lon units");
        strcpy(str_att, "grid_longitude");
        if ((retval = nc_put_att_text(ncid, vRlon_id, "standard_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lon standard_name");
        // Lat
        if ((retval = nc_def_var(ncid, "lat", NC_FLOAT, 1, dRlat_id, &vRlat_id)))
          ERR(retval, "lat");
        strcpy(str_att, "latitude");
        if ((retval = nc_put_att_text(ncid, vRlat_id, "long_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lat long_name");
        strcpy(str_att, "degrees");
        if ((retval = nc_put_att_text(ncid, vRlat_id, "units", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lat units");
        strcpy(str_att, "grid_latitude");
        if ((retval = nc_put_att_text(ncid, vRlat_id, "standard_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lat standard_name");
      } else {
        // Rotated lon
        if ((retval = nc_def_var(ncid, "rlon", NC_FLOAT, 1, dRlon_id, &vRlon_id)))
          ERR(retval, "rlon");
        strcpy(str_att, "longitude in rotated_pole grid");
        if ((retval = nc_put_att_text(ncid, vRlon_id, "long_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: rlon long_name");
        strcpy(str_att, "degrees");
        if ((retval = nc_put_att_text(ncid, vRlon_id, "units", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: rlon units");
        strcpy(str_att, "grid_longitude");
        if ((retval = nc_put_att_text(ncid, vRlon_id, "standard_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: rlon standard_name");
        // Rotated lat
        if ((retval = nc_def_var(ncid, "rlat", NC_FLOAT, 1, dRlat_id, &vRlat_id)))
          ERR(retval, "rlat");
        strcpy(str_att, "latitude in rotated_pole grid");
        if ((retval = nc_put_att_text(ncid, vRlat_id, "long_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: rlat long_name");
        strcpy(str_att, "degrees");
        if ((retval = nc_put_att_text(ncid, vRlat_id, "units", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: rlat units");
        strcpy(str_att, "grid_latitude");
        if ((retval = nc_put_att_text(ncid, vRlat_id, "standard_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: rlat standard_name");
      }
      // Levels
      if ((retval = nc_def_var(ncid, "lm", NC_INT, 1, dLm_id, &vLm_id))) ERR(retval, "lm");
      strcpy(str_att, "unitless");
      if ((retval = nc_put_att_text(ncid, vLm_id, "units", strlen(str_att),
                                    &str_att[0])))
        ERR(retval, "Attr: lm units");
      strcpy(str_att, "layer id");
      if ((retval = nc_put_att_text(ncid, vLm_id, "long_name", strlen(str_att),
                                    &str_att[0])))
        ERR(retval, "Attr: lm long_name");
      strcpy(str_att, "down");
      if ((retval = nc_put_att_text(ncid, vLm_id, "positive", strlen(str_att),
                                    &str_att[0])))
        ERR(retval, "Attr: lm positive");
      if (has_int_press) {
        // Interface pressure levels
        if ((retval = nc_def_var(ncid, "lmp", NC_INT, 1, dLmp_id, &vLmp_id)))
          ERR(retval, "lmp");
        strcpy(str_att, "unitless");
        if ((retval = nc_put_att_text(ncid, vLmp_id, "units", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lmp units");
        strcpy(str_att, "interface layer id");
        if ((retval = nc_put_att_text(ncid, vLmp_id, "long_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lmp long_name");
        strcpy(str_att, "down");
        if ((retval = nc_put_att_text(ncid, vLmp_id, "positive", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lmp positive");
      }
      if (!isGlobal) {
        // Rotated pole
        if ((retval = nc_def_var(ncid, "rotated_pole", NC_CHAR, 0, dLm_id, &vrp_id)))
          ERR(retval,
         "rotated_pole");
        strcpy(str_att, "rotated_latitude_longitude");
        if ((retval = nc_put_att_text(ncid, vrp_id, "grid_mapping_name", strlen(str_att),
          &str_att[0]))) ERR(retval, "Attr: rotated_pole grid_mapping_name");
        double pole[2];
        pole[0] = 0.;
        pole[1] = 90.;
        geom.grid().projection().xy2lonlat(pole);
        float_att = pole[1];
        if ((retval = nc_put_att_float(ncid, vrp_id, "grid_north_pole_latitude", NC_FLOAT, 1,
          &float_att))) ERR(retval, "Attr: rotated_pole grid_north_pole_latitude");
        float_att = pole[0];
        if (float_att > 180.) {
          float_att -= 360.;
        }
        if ((retval = nc_put_att_float(ncid, vrp_id, "grid_north_pole_longitude", NC_FLOAT, 1,
               &float_att))) ERR(retval, "Attr: rotated_pole grid_north_pole_longitude");
      }
      // Time steps
      if ((retval = nc_def_var(ncid, "time", NC_INT, 1, dTime_id, &vTime_id)))
        ERR(retval, "time");
      strcpy(str_att,
        ("hours since "+geom.io().getString("initial date").substr(0, 10)+
        " "+geom.io().getString("initial date").substr(11, 5)+" UTC").c_str());
      if ((retval = nc_put_att_text(ncid, vTime_id, "units", strlen(str_att),
                                    &str_att[0])))
        ERR(retval, "Attr: time units");
      strcpy(str_att, "time");
      if ((retval = nc_put_att_text(ncid, vTime_id, "long_name", strlen(str_att),
                                    &str_att[0])))
        ERR(retval, "Attr: time long_name");
      strcpy(str_att, "standard");
      if ((retval = nc_put_att_text(ncid, vTime_id, "calendar", strlen(str_att),
                                    &str_att[0])))
        ERR(retval, "Attr: time calendar");
      strcpy(str_att, "time");
      if ((retval = nc_put_att_text(ncid, vTime_id, "standard_name", strlen(str_att),
                                    &str_att[0])))
        ERR(retval, "Attr: time standard_name");
      if (!isGlobal) {
        // Geographic lon
        if ((retval = nc_def_var(ncid, "lon", NC_FLOAT, 2, d2D_id, &lon_id)))
          ERR(retval, "lon");
        strcpy(str_att, "longitude");
        if ((retval = nc_put_att_text(ncid, lon_id, "long_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lon long_name");
        strcpy(str_att, "degrees_east");
        if ((retval = nc_put_att_text(ncid, lon_id, "units", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lon units");
        strcpy(str_att, "longitude");
        if ((retval = nc_put_att_text(ncid, lon_id, "standard_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lon standard_name");
        float_att = -999999.0;
        if ((retval = nc_put_att_float(ncid, lon_id, "missing_value", NC_FLOAT, 1,
                                       &float_att)))
          ERR(retval, "Attr: lon missing_value");
        float_att = -32767.0;
        if ((retval = nc_put_att_float(ncid, lon_id, "_FillValue", NC_FLOAT, 1,
                                       &float_att)))
          ERR(retval, "Attr: lon _FillValue");
        strcpy(str_att, "lon lat");
        if ((retval = nc_put_att_text(ncid, lon_id, "coordinates", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lon coordinates");
        // Geographic lat
        if ((retval = nc_def_var(ncid, "lat", NC_FLOAT, 2, d2D_id, &lat_id)))
          ERR(retval, "lat");
        strcpy(str_att, "latitude");
        if ((retval = nc_put_att_text(ncid, lat_id, "long_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lat long_name");
        strcpy(str_att, "degrees_north");
        if ((retval = nc_put_att_text(ncid, lat_id, "units", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lat units");
        strcpy(str_att, "latitude");
        if ((retval = nc_put_att_text(ncid, lat_id, "standard_name", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lat standard_name");
        float_att = -999999.0;
        if ((retval = nc_put_att_float(ncid, lat_id, "missing_value", NC_FLOAT, 1,
                                       &float_att)))
          ERR(retval, "Attr: lat missing_value");
        float_att = -32767.0;
        if ((retval = nc_put_att_float(ncid, lat_id, "_FillValue", NC_FLOAT, 1,
                                       &float_att)))
          ERR(retval, "Attr: lat _FillValue");
        strcpy(str_att, "lon lat");
        if ((retval = nc_put_att_text(ncid, lat_id, "coordinates", strlen(str_att),
                                      &str_att[0])))
          ERR(retval, "Attr: lat coordinates");
      }
    }

    // Get attributes for this grid uid
    ASSERT(attributes_.has(geom.grid().uid()));
    const eckit::LocalConfiguration gridAttr(attributes_, geom.grid().uid());

    for (size_t jvar = 0; jvar < vars.size(); ++jvar) {
      for (const auto & item : geom.alias()) {
        if (item.getString("in file") == vars[jvar]) {
          var_in_code = item.getString("in code");
        }
      }
      // Check whether this variable exists
      if (nc_inq_varid(ncid, vars[jvar].c_str(), &var_id[jvar]) != NC_NOERR) {
        // Define variable
        if (fields.fieldSet()[vars[jvar]].shape(1) > 1) {
          if (has_int_press && geom.levels(var_in_code) > *lev_set.begin()) {
            if ((retval = nc_def_var(ncid, vars[jvar].c_str(), NC_FLOAT, 4, d4Dp_id,
              &var_id[jvar]))) ERR(retval, vars[jvar]);
          } else {
            if ((retval = nc_def_var(ncid, vars[jvar].c_str(), NC_FLOAT, 4, d4D_id, &var_id[jvar])))
              ERR(retval, vars[jvar]);
          }
        } else {
          if ((retval = nc_def_var(ncid, vars[jvar].c_str(), NC_FLOAT, 3, d3D_id, &var_id[jvar])))
            ERR(retval, vars[jvar]);
        }

        // Get attributes list for this variable
        ASSERT(gridAttr.has(vars[jvar]));
        const auto & varAttrs = gridAttr.getSubConfigurations(vars[jvar]);

        // Write attributes
        for (const auto & attr : varAttrs) {
          // Get attributes name/type/tokens
          const std::string aname = attr.getString("name");
          const nc_type atype = static_cast<nc_type>(attr.getInt("type"));
          const std::vector<std::string> tokens = attr.getStringVector("tokens");

          if (atype == NC_CHAR) {
            const std::string & txt = (tokens.empty() ? std::string() : tokens[0]);
            if ((retval = nc_put_att_text(ncid, var_id[jvar], aname.c_str(), txt.size(),
                                          txt.c_str())))
              ERR(retval, ("Attr write text:"+aname).c_str());
          } else if (atype == NC_INT) {
            std::vector<int> buf(tokens.size());
            for (size_t k = 0; k < tokens.size(); ++k) buf[k] = std::stoi(tokens[k]);
            if ((retval = nc_put_att_int(ncid, var_id[jvar], aname.c_str(), NC_INT, buf.size(),
                                         buf.data())))
              ERR(retval, ("Attr write int:"+aname).c_str());
          } else if (atype == NC_FLOAT) {
            std::vector<float> buf(tokens.size());
            for (size_t k = 0; k < tokens.size(); ++k) buf[k] = std::stof(tokens[k]);
            if ((retval = nc_put_att_float(ncid, var_id[jvar], aname.c_str(), NC_FLOAT, buf.size(),
                                           buf.data())))
              ERR(retval, ("Attr write float:"+aname).c_str());
          } else if (atype == NC_DOUBLE) {
            std::vector<double> buf(tokens.size());
            for (size_t k = 0; k < tokens.size(); ++k) buf[k] = std::stod(tokens[k]);
            if ((retval = nc_put_att_double(ncid, var_id[jvar], aname.c_str(),
                                            NC_DOUBLE, buf.size(), buf.data())))
              ERR(retval, ("Attr write double:"+aname).c_str());
          } else if (atype == NC_SHORT) {
            std::vector<int16_t> buf(tokens.size());
            for (size_t k = 0; k < tokens.size(); ++k) buf[k] =
              static_cast<int16_t>(std::stoi(tokens[k]));
            if ((retval = nc_put_att_short(ncid, var_id[jvar], aname.c_str(), NC_SHORT, buf.size(),
                                           buf.data())))
              ERR(retval, ("Attr write short:"+aname).c_str());
          } else {
            std::string joined;
            for (size_t i = 0; i < tokens.size(); ++i) {
              if (i) joined += ",";
              joined += tokens[i];
            }
            if ((retval = nc_put_att_text(ncid, var_id[jvar], aname.c_str(), joined.size(),
                                          joined.c_str())))
              ERR(retval, ("Attr write fallback text:"+aname).c_str());
          }
        }
      }
    }

    // End definition mode
    if ((retval = nc_enddef(ncid))) ERR(retval, ncFilePath);

    // Data mode

    if (!existingFile) {
      std::vector<float> zRlon(nx_out);
      std::vector<float> zRlat(ny);
      std::vector<float> zlon(ny*nx);
      std::vector<float> zlat(ny*nx);
      // Create (r)lon
      const float rlonStart = grid.spec().getFloat("xspace.start");
      const float rlonEnd = grid.spec().getFloat("xspace.end");
      for (size_t jLon = 0; jLon < nx_out; ++jLon) {
        zRlon[jLon] = rlonStart + static_cast<float>(jLon)*(rlonEnd-rlonStart)
          /static_cast<float>(nx_out-1);
      }

      // Create (r)lat
      float rlatStart = grid.spec().getFloat("yspace.start");
      float rlatEnd = grid.spec().getFloat("yspace.end");
      for (size_t jLat = 0; jLat < ny; ++jLat) {
        zRlat[jLat] = rlatStart + static_cast<float>(jLat)*(rlatEnd-rlatStart)
          /static_cast<float>(ny-1);
      }

      if (!isGlobal) {
        // Copy coordinates
        const auto lonViewGlobal = atlas::array::make_view<double, 1>(globalData["lon"]);
        const auto latViewGlobal = atlas::array::make_view<double, 1>(globalData["lat"]);
        for (size_t j = 0; j < ny; ++j) {
          const atlas::idx_t jj = ny-1-j;
          for (atlas::idx_t i = 0; i < grid.nx(jj); ++i) {
            atlas::gidx_t gidx = grid.index(i, jj);
            zlon[jj*nx+i] = lonViewGlobal(gidx);
            zlat[jj*nx+i] = latViewGlobal(gidx);
          }
        }
      }

      // Create lm
      std::vector<int> zLm(lmMax);
      for (size_t jLm = 0; jLm < lmMax; ++jLm) {
        zLm[jLm] = jLm;
      }

      // Create lmp
      std::vector<int> zLmp(lmMax+1);
      if (has_int_press) {
        for (size_t jLmp = 0; jLmp < lmMax+1; ++jLmp) {
          zLmp[jLmp] = jLmp;
        }
      }

      // Create time
      std::vector<int> zTime(timeMax);
      for (size_t jTime = 0; jTime < timeMax; ++jTime) {
        zTime[jTime] = jTime*timeStep/3600 + timeOffset;
      }

      // Write coordinates
      if ((retval = nc_put_var_float(ncid, vRlon_id, zRlon.data()))) ERR(retval, "(r)lon");
      if ((retval = nc_put_var_float(ncid, vRlat_id, zRlat.data()))) ERR(retval, "(r)lat");
      if (!isGlobal) {
        if ((retval = nc_put_var_float(ncid, lon_id, zlon.data()))) ERR(retval, "lon");
        if ((retval = nc_put_var_float(ncid, lat_id, zlat.data()))) ERR(retval, "lat");
      }
      if ((retval = nc_put_var_int(ncid, vLm_id, zLm.data()))) ERR(retval, "lm");
      if (has_int_press) {
        if ((retval = nc_put_var_int(ncid, vLmp_id, zLmp.data()))) ERR(retval, "lmp");
      }
      const std::vector<size_t> startp({0});
      const std::vector<size_t> countp({timeMax});
      if ((retval = nc_put_vars_int(ncid, vTime_id, startp.data(), countp.data(), NULL,
        zTime.data()))) ERR(retval, "time");
    }

    for (size_t jvar = 0; jvar < vars.size(); ++jvar) {
      // Get variable view
      const auto varView = atlas::array::make_view<double, 2>(globalData[vars[jvar]]);

      // Get transformation parameters (for states only)
      double scaleFactor = 1.0;
      bool logTransf = false;
      double addConst = 0.0;
      if (isState) {
        for (const auto & item : geom.alias()) {
          if (item.getString("in file") == vars[jvar]) {
            scaleFactor = item.getDouble("scaling factor", 1.0);
            logTransf = item.getBool("log transform", false);
            if (logTransf) {
              addConst = item.getDouble("additive constant", 0.0);
            }
          }
        }
      }

      if (fields.fieldSet()[vars[jvar]].shape(1) == 1) {
        // Copy data
        std::vector<float> zvar(ny*nx_out);
        for (size_t j = 0; j < ny; ++j) {
          const atlas::idx_t jj = ny-1-j;
          for (atlas::idx_t i = 0; i < grid.nx(jj); ++i) {
            atlas::gidx_t gidx = grid.index(i, jj);
            if (logTransf) {
              zvar[jj*nx_out+i] = (pow(10, varView(gidx, 0)) - addConst) / scaleFactor;
            } else {
              zvar[jj*nx_out+i] = varView(gidx, 0) / scaleFactor;
            }
          }
          if (isGlobal) {
            zvar[jj*nx_out+grid.nx(jj)] = zvar[jj*nx_out];
          }
        }

        // Write data
        const std::vector<size_t> startp({time, 0, 0});
        const std::vector<size_t> countp({1, ny, nx_out});
        if ((retval = nc_put_vars_float(ncid, var_id[jvar], startp.data(), countp.data(), NULL,
                                        zvar.data()))) ERR(retval, vars[jvar]);
      } else {
        for (const auto & item : geom.alias()) {
          if (item.getString("in file") == vars[jvar]) {
            var_in_code = item.getString("in code");
          }
        }
        auto levels = geom.vertCoordAvg(var_in_code);
        for (size_t k = 0; k < levels.size(); ++k) {
          // Copy data
          std::vector<float> zvar(ny*nx_out);
          for (size_t j = 0; j < ny; ++j) {
            const atlas::idx_t jj = ny-1-j;
            for (atlas::idx_t i = 0; i < grid.nx(jj); ++i) {
              atlas::gidx_t gidx = grid.index(i, jj);
              if (logTransf) {
                zvar[jj*nx_out+i] = (pow(10, varView(gidx, k)) - addConst) / scaleFactor;
              } else {
                zvar[jj*nx_out+i] = varView(gidx, k) / scaleFactor;
              }
            }
            if (isGlobal) {
              zvar[jj*nx_out+grid.nx(jj)] = zvar[jj*nx_out];
            }
          }

          // Write data
          const std::vector<size_t> countp({1, 1, ny, nx_out});
          const std::vector<size_t> startp({time, static_cast<size_t>(levels[k]), 0, 0});
          if ((retval = nc_put_vars_float(ncid, var_id[jvar], startp.data(), countp.data(),
                NULL, zvar.data()))) ERR(retval, vars[jvar]);
        }
      }
    }

    // Close file
    if ((retval = nc_close(ncid))) ERR(retval, ncFilePath);
  }

  oops::Log::trace() << classname() << "::write done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
