/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "vind/FieldsIO/FieldsIOAQ.h"

#include <netcdf.h>
#include <stdint.h>

#include <cmath>
#include <memory>
#include <unordered_map>
#include <vector>

#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"

#define ERR(e, msg) {std::string s(nc_strerror(e)); \
  throw eckit::Exception(s + " : " + msg, Here());}

namespace vind {

// -----------------------------------------------------------------------------

static FieldsIOMaker<FieldsIOAQ> makerAQ_("aq");

// -----------------------------------------------------------------------------

static std::vector<std::string> existingFiles_;
static eckit::LocalConfiguration attributes_;

// -----------------------------------------------------------------------------

void FieldsIOAQ::read(const oops::Variables & vars,
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

  // Get file reference time
  const util::DateTime initialTime(geom.io().getString("initial date"));

  // Get read time
  const util::DateTime validTime(conf.getString("date"));

  // Files are single date: the time counter is always 0
  const size_t time = 0;

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
    const std::string initialTimeFromFileStr = time_units_value.substr(14, 10) + "T"
     + time_units_value.substr(25, 5) + ":00Z";
    const util::DateTime initialTimeFromFile(initialTimeFromFileStr);

    // Check that state time and model time are the same
    float modtime;
    if ((retval = nc_get_var_float(ncid, time_id, &modtime))) ERR(retval, "time");
    ASSERT((validTime-initialTimeFromFile).toSeconds() == static_cast<int64_t>(modtime));

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

      // Get variable in code
      const std::string var_in_code = geom.params().codeAlias(vars[jvar].name());

      // Get transformation parameters (for states only)
      double scalingFactor = 1.0;
      bool logTransf = false;
      double addConst = 0.0;
      if (isState) {
        scalingFactor = geom.params().scalingFactor(var_in_code);
        logTransf = geom.params().logTransf(var_in_code);
        addConst = geom.params().addConst(var_in_code);
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
              varView(gidx, 0) = log10((zvar[jj*nx+i] * scalingFactor) + addConst);
            } else {
              varView(gidx, 0) = zvar[jj*nx+i] * scalingFactor;
            }
          }
        }
      } else {
        const size_t nlev = fields.fieldSet()[vars[jvar].name()].shape(1);
        for (size_t k = 0; k < nlev; ++k) {
          // Read level
          std::vector<double> zvar(nx*ny);
          const std::vector<size_t> startp({time, k, 0, 0});
          const std::vector<size_t> countp({1, 1, ny, nx});
          if ((retval = nc_get_vars_double(ncid, var_id[jvar], startp.data(), countp.data(), NULL,
            zvar.data()))) ERR(retval, vars[jvar].name());

          // Deserialize data to view
          for (size_t j = 0; j < ny; ++j) {
            const atlas::idx_t jj = ny-1-j;
            for (atlas::idx_t i = 0; i < grid.nx(jj); ++i) {
              atlas::gidx_t gidx = grid.index(i, jj);
              if (logTransf) {
                varView(gidx, k) = log10((zvar[jj*nx+i] * scalingFactor) + addConst);
              } else {
                varView(gidx, k) = zvar[jj*nx+i] * scalingFactor;
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

void FieldsIOAQ::write(const eckit::Configuration & conf,
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

  // Check if this file already exists
  const bool existingFile = std::find(existingFiles_.begin(), existingFiles_.end(), ncFilePath)
    != existingFiles_.end();
  if (!existingFile) {
    existingFiles_.push_back(ncFilePath);
  }

  // Get total number of levels (from geometry section)
  const size_t lmMax = fields.fieldSet()[vars[0]].shape(1);

  // Get write time
  const util::DateTime validTime(conf.getString("date"));

  // Reference for time coordinate
  int64_t timeOffset =
    (validTime- util::DateTime(geom.io().getString("initial date"))).toSeconds();

  // Single date file
  const size_t time = 0;

  // NetCDF IDs
  int retval, ncid, lon_id, lat_id, lev_id, time_id,
    dlon_id[1], dlat_id[1], dlev_id[1], dTime_id[1],
    d3D_id[3], d4D_id[4],
    vlon_id, vlat_id, vlev_id, vTime_id,
    var_id[vars.size()];

  // Prepare local coordinates and data
  atlas::FieldSet localData;
  if (!existingFile) {
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

  // Prepare global coordinates and data
  atlas::FieldSet globalData;
  if (!existingFile) {
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
      if ((retval = nc_inq_dimid(ncid, "lon", &lon_id))) ERR(retval, "lon");
      if ((retval = nc_inq_dimid(ncid, "lat", &lat_id))) ERR(retval, "lat");
      if ((retval = nc_inq_dimid(ncid, "lev", &lev_id))) ERR(retval, "lev");
      if ((retval = nc_inq_dimid(ncid, "time", &time_id))) ERR(retval, "time");
    } else {
      // Create dimensions
      if ((retval = nc_def_dim(ncid, "lon", nx, &lon_id))) ERR(retval, "lon");
      if ((retval = nc_def_dim(ncid, "lat", ny, &lat_id))) ERR(retval, "lat");
      if ((retval = nc_def_dim(ncid, "lev", lmMax, &lev_id))) ERR(retval, "lev");
      if ((retval = nc_def_dim(ncid, "time", 1, &time_id))) ERR(retval, "time");
    }

    // Dimensions arrays
    dlon_id[0] = lon_id;
    dlat_id[0] = lat_id;
    dlev_id[0] = lev_id;
    dTime_id[0] = time_id;
    d3D_id[0] = time_id;
    d3D_id[1] = lat_id;
    d3D_id[2] = lon_id;
    d4D_id[0] = time_id;
    d4D_id[1] = lev_id;
    d4D_id[2] = lat_id;
    d4D_id[3] = lon_id;

    // Attributes storage
    char str_att[128];

    if (!existingFile) {
      // Define coordinates
      // lon
      if ((retval = nc_def_var(ncid, "lon", NC_FLOAT, 1, dlon_id, &vlon_id)))
        ERR(retval, "lon");
      strcpy(str_att, "degrees_east");
      if ((retval = nc_put_att_text(ncid, vlon_id, "units", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lon units");
      strcpy(str_att, "X");
      if ((retval = nc_put_att_text(ncid, vlon_id, "axis", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lon axis");
      strcpy(str_att, "longitude");
      if ((retval = nc_put_att_text(ncid, vlon_id, "standard_name", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lon standard_name");
      strcpy(str_att, "linear");
      if ((retval = nc_put_att_text(ncid, vlon_id, "realtopology", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lon realtopology");
      // lat
      if ((retval = nc_def_var(ncid, "lat", NC_FLOAT, 1, dlat_id, &vlat_id)))
        ERR(retval, "lat");
      strcpy(str_att, "degrees_north");
      if ((retval = nc_put_att_text(ncid, vlat_id, "units", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lat units");
      strcpy(str_att, "Y");
      if ((retval = nc_put_att_text(ncid, vlat_id, "axis", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lat axis");
      strcpy(str_att, "latitude");
      if ((retval = nc_put_att_text(ncid, vlat_id, "standard_name", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lat standard_name");
      strcpy(str_att, "linear");
      if ((retval = nc_put_att_text(ncid, vlat_id, "realtopology", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lat realtopology");
      // Levels
      if ((retval = nc_def_var(ncid, "lev", NC_INT, 1, dlev_id, &vlev_id)))
        ERR(retval, "lev");
      strcpy(str_att, "dimensionless");
      if ((retval = nc_put_att_text(ncid, vlev_id, "units", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lev units");
      strcpy(str_att, "Z");
      if ((retval = nc_put_att_text(ncid, vlev_id, "axis", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lev axis");
      strcpy(str_att, "atmosphere_hybrid_sigma_pressure_coordinate");
      if ((retval = nc_put_att_text(ncid, vlev_id, "standard_name", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lev standard_name");
      strcpy(str_att, "down");
      if ((retval = nc_put_att_text(ncid, vlev_id, "positive", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lev positive");
      strcpy(str_att, "linear");
      if ((retval = nc_put_att_text(ncid, vlev_id, "realtopology", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: lev realtopology");
      // Time steps
      if ((retval = nc_def_var(ncid, "time", NC_FLOAT, 1, dTime_id, &vTime_id)))
        ERR(retval, "time");
      strcpy(str_att,
        ("seconds since "+geom.io().getString("initial date").substr(0, 10)+
        " "+geom.io().getString("initial date").substr(11, 5)).c_str());
      if ((retval = nc_put_att_text(ncid, vTime_id, "units", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: time units");
      strcpy(str_att, "t");
      if ((retval = nc_put_att_text(ncid, vTime_id, "axis", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: time axis");
      strcpy(str_att, (geom.io().getString("initial date").substr(0, 10)+
        " "+geom.io().getString("initial date").substr(11, 5)).c_str());
      if ((retval = nc_put_att_text(ncid, vTime_id, "time_origin", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: time time_origin");
      strcpy(str_att, "standard");
      if ((retval = nc_put_att_text(ncid, vTime_id, "calendar", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: time calendar");
      strcpy(str_att, "Time axis");
      if ((retval = nc_put_att_text(ncid, vTime_id, "standard_name", strlen(str_att), &str_att[0])))
        ERR(retval, "Attr: time standard_name");
    }

    // Get attributes for this grid uid
    ASSERT(attributes_.has(geom.grid().uid()));
    const eckit::LocalConfiguration gridAttr(attributes_, geom.grid().uid());

    for (size_t jvar = 0; jvar < vars.size(); ++jvar) {
      // Check whether this variable exists
      if (nc_inq_varid(ncid, vars[jvar].c_str(), &var_id[jvar]) != NC_NOERR) {
        // Define variable
        if (fields.fieldSet()[vars[jvar]].shape(1) > 1) {
          if ((retval = nc_def_var(ncid, vars[jvar].c_str(), NC_FLOAT, 4, d4D_id, &var_id[jvar])))
            ERR(retval, vars[jvar]);
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
      // Create lon
      std::vector<float> zlon(nx);
      const float lonStart = grid.spec().getFloat("xspace.start");
      const float lonEnd = grid.spec().getFloat("xspace.end");
      for (size_t jLon = 0; jLon < nx; ++jLon) {
        zlon[jLon] = lonStart + static_cast<float>(jLon)*(lonEnd-lonStart)
          /static_cast<float>(nx-1);
      }

      // Create rlat
      std::vector<float> zlat(ny);
      const float latStart = grid.spec().getFloat("yspace.start");
      const float latEnd = grid.spec().getFloat("yspace.end");
      for (size_t jLat = 0; jLat < ny; ++jLat) {
        zlat[jLat] = latStart + static_cast<float>(jLat)*(latEnd-latStart)
          /static_cast<float>(ny-1);
      }

      // Recover geometry levels
      std::string vunits;
      std::vector<double> zlev = geom.verticalCoord(vunits);
      std::vector<int> zLm(lmMax);
      for (size_t jLm = 0; jLm < lmMax; ++jLm) {
        zLm[jLm] = static_cast<int>(zlev[jLm]);
      }

      // Create time
      std::vector<float> zTime(1);
      zTime[0] = static_cast<float>(timeOffset);

      // Write coordinates
      if ((retval = nc_put_var_float(ncid, vlon_id, zlon.data()))) ERR(retval, "lon");
      if ((retval = nc_put_var_float(ncid, vlat_id, zlat.data()))) ERR(retval, "lat");
      if ((retval = nc_put_var_int(ncid, vlev_id, zLm.data()))) ERR(retval, "lev");
      if ((retval = nc_put_var_float(ncid, vTime_id, zTime.data()))) ERR(retval, "time");
    }

    for (size_t jvar = 0; jvar < vars.size(); ++jvar) {
      // Get variable view
      const auto varView = atlas::array::make_view<double, 2>(globalData[vars[jvar]]);

      // Get variable in code
      const std::string var_in_code = geom.params().codeAlias(vars[jvar]);

      // Get transformation parameters (for states only)
      double scalingFactor = 1.0;
      bool logTransf = false;
      double addConst = 0.0;
      if (isState) {
        scalingFactor = geom.params().scalingFactor(var_in_code);
        logTransf = geom.params().logTransf(var_in_code);
        addConst = geom.params().addConst(var_in_code);
      }

      if (fields.fieldSet()[vars[jvar]].shape(1) == 1) {
        // Copy data
        std::vector<float> zvar(ny*nx);
        for (size_t j = 0; j < ny; ++j) {
          const atlas::idx_t jj = ny-1-j;
          for (atlas::idx_t i = 0; i < grid.nx(jj); ++i) {
            atlas::gidx_t gidx = grid.index(i, jj);
            if (logTransf) {
              zvar[jj*nx+i] = (pow(10, varView(gidx, 0)) - addConst) / scalingFactor;
            } else {
              zvar[jj*nx+i] = varView(gidx, 0) / scalingFactor;
            }
          }
        }

        // Write data
        const std::vector<size_t> startp({time, 0, 0});
        const std::vector<size_t> countp({1, ny, nx});
        if ((retval = nc_put_vars_float(ncid, var_id[jvar], startp.data(), countp.data(), NULL,
                                        zvar.data()))) ERR(retval, vars[jvar]);
      } else {
        for (atlas::idx_t k = 0; k < fields.fieldSet()[vars[jvar]].shape(1); ++k) {
          // Copy data
          std::vector<float> zvar(ny*nx);
          for (size_t j = 0; j < ny; ++j) {
            const atlas::idx_t jj = ny-1-j;
            for (atlas::idx_t i = 0; i < grid.nx(jj); ++i) {
              atlas::gidx_t gidx = grid.index(i, jj);
              if (logTransf) {
                zvar[jj*nx+i] = (pow(10, varView(gidx, k)) - addConst) / scalingFactor;
              } else {
                zvar[jj*nx+i] = varView(gidx, k) / scalingFactor;
              }
            }
          }

          // Write data
          const std::vector<size_t> countp({1, 1, ny, nx});
          const std::vector<size_t> startp({time, size_t(k), 0, 0});
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
