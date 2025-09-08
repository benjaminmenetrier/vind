/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/FieldsIO/FieldsIOArome.h"

#include <netcdf.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"

#include "oops/util/Logger.h"

#ifdef READFA
#include "vind/FieldsIO/fieldsio_arome_fa.h"
#endif
#include "vind/Fields.h"

#define ERR(e, msg) {std::string s(nc_strerror(e)); \
  throw eckit::Exception(s + " : " + msg, Here());}

namespace vind {

// -----------------------------------------------------------------------------

static FieldsIOMaker<FieldsIOArome> makerAromeNetCDF_("arome netcdf");
static FieldsIOMaker<FieldsIOArome> makerAromeFA_("arome fa");

// -----------------------------------------------------------------------------

#ifdef READFA
static struct Trans_t trans;
static inline bool transSetup = false;
#endif

// -----------------------------------------------------------------------------

void FieldsIOArome::read(const oops::Variables & vars,
                         const eckit::Configuration & conf,
                         Fields & fields) const {
  oops::Log::trace() << classname() << "::read starting" << std::endl;

  // StructuredColumns
  atlas::functionspace::StructuredColumns fs(fields.geometry()->functionSpace());

  // Get grid
  atlas::StructuredGrid grid = fs.grid();

  // Get file path
  std::string filePath = conf.getString("filepath");
  if (conf.has("member")) {
    std::ostringstream out;
    out << std::setfill('0') << std::setw(6) << conf.getInt("member");
    filePath.append("_");
    filePath.append(out.str());
  }

  // NetCDF file path
  if (ioFormat_ == "arome netcdf") {
    filePath = filePath + ".nc";
  }

  // Clear local fieldset
  fields.fieldSet().clear();

  // Variables to copy / to read
  oops::Variables varsToRead;
  for (const auto & var : vars) {
    if (var.name() == "log_of_air_pressure_at_surface" || var.name() == "air_pressure_at_surface"
      || var.name() == "air_pressure" || var.name() == "air_pressure_at_half_levels") {
      // Get surface pressure
      varsToRead.push_back("SURFPRESSION");
      varsToRead["SURFPRESSION"].setLevels(1);
    } else if (var.name() == "height_above_mean_sea_level_at_surface") {
      // Get surface geopotential and retrieve surface height
      varsToRead.push_back("SPECSURFGEOPOTEN");
      varsToRead["SPECSURFGEOPOTEN"].setLevels(1);
    } else if (var.name() == "geographical_x_wind" || var.name() == "eastward_wind") {
      // Get u wind
      varsToRead.push_back("WIND.U.PHYS");
      varsToRead["WIND.U.PHYS"].setLevels(var.getLevels());
    } else if (var.name() == "geographical_y_wind" || var.name() == "northward_wind") {
      // Get v wind
      varsToRead.push_back("WIND.V.PHYS");
      varsToRead["WIND.V.PHYS"].setLevels(var.getLevels());
    } else if (var.name() == "air_temperature") {
      // Get temperature
      varsToRead.push_back("TEMPERATURE");
      varsToRead["TEMPERATURE"].setLevels(var.getLevels());
    } else if (var.name() == "water_vapor_mixing_ratio_wrt_moist_air") {
      // Get specific humidity
      varsToRead.push_back("HUMI.SPECIFI");
      varsToRead["HUMI.SPECIFI"].setLevels(var.getLevels());
    } else {
      // Unknown variable
      throw eckit::Exception("unknown variable: " + var.name(), Here());
    }
  }

  // Create local fieldset
  atlas::FieldSet fsetToRead;
  for (const auto & var : varsToRead) {
    atlas::Field field = fs.createField<double>(
      atlas::option::name(var.name()) | atlas::option::levels(var.getLevels()));
    fsetToRead.add(field);
  }

  // Initialize local fieldset
  for (auto & field : fsetToRead) {
    auto view = atlas::array::make_view<double, 2>(field);
    view.assign(0.0);
  }

  // Global data
  atlas::FieldSet globalData;
  for (const auto & var : varsToRead) {
    atlas::Field field = fs.createField<double>(atlas::option::name(var.name())
     | atlas::option::levels(var.getLevels()) | atlas::option::global());
    globalData.add(field);
  }

  // Hybrid coordinates dimension
  size_t nab;

  // Define hybrid coordinates
  std::vector<double> akFromFile;
  std::vector<double> bkFromFile;

  // File variables names
  size_t nVar2D = 0;
  std::vector<std::string> preVec;
  std::vector<int> levVec;
  std::vector<std::string> varVec;
  for (const auto & var : varsToRead) {
    for (int jlevel = 0; jlevel < var.getLevels(); ++jlevel) {
      if (var.name() == "SURFPRESSION") {
        preVec.push_back("SURF");
        levVec.push_back(0);
        varVec.push_back("PRESSION");
      } else if (var.name() == "SPECSURFGEOPOTEN") {
        preVec.push_back("SPECSURF");
        levVec.push_back(0);
        varVec.push_back("GEOPOTENTIEL");
      } else {
        preVec.push_back("S");
        levVec.push_back(jlevel+1);
        varVec.push_back(var.name());
      }
      ++nVar2D;
    }
  }

  // Get sizes
  const atlas::util::Config xspec = grid.xspace().spec();
  const atlas::util::Config yspec = grid.yspace().spec();
  const size_t nx = xspec.getInt("N");
  const size_t ny = yspec.getInt("N");

  oops::Log::info() << "Info     : Reading file: " << filePath << std::endl;

  if (ioFormat_ == "arome netcdf") {
    // NetCDF IDs
    int ncid, retval, ak_id, bk_id, dim_id, var_id[nVar2D];

    // Variable/level name
    std::vector<std::string> var2DName;
    for (size_t jVar2D = 0; jVar2D < nVar2D; ++jVar2D) {
      if (levVec[jVar2D] == 0) {
        var2DName.push_back(preVec[jVar2D] + varVec[jVar2D]);
      } else {
        const std::string level = std::to_string(levVec[jVar2D]);
        var2DName.push_back(preVec[jVar2D] + std::string(3-level.length(), '0')
          + level + varVec[jVar2D]);
      }
    }

    if (fields.geometry()->getComm().rank() == 0) {
      // Open NetCDF file
      if ((retval = nc_open(filePath.c_str(), NC_NOWRITE, &ncid))) ERR(retval, filePath);

      // Get hybrid coordinates IDs
      if ((retval = nc_inq_varid(ncid, "hybrid_coef_A", &ak_id))) ERR(retval, "hybrid_coef_A");
      if ((retval = nc_inq_varid(ncid, "hybrid_coef_B", &bk_id))) ERR(retval, "hybrid_coef_B");

      // Get hybrid coordinates dimension
      if ((retval = nc_inq_vardimid(ncid, ak_id, &dim_id))) ERR(retval, "hybrid_coef_A");
      if ((retval = nc_inq_dimlen(ncid, dim_id, &nab))) ERR(retval, "nab");

      // Get variables IDs
      for (size_t jVar2D = 0; jVar2D < nVar2D; ++jVar2D) {
        if ((retval = nc_inq_varid(ncid, var2DName[jVar2D].c_str(), &var_id[jVar2D]))) {
          ERR(retval, var2DName[jVar2D]);
        }
      }
    }

    // Broadcast hybrid coordinates dimension
    fields.geometry()->getComm().broadcast(nab, 0);

    // Allocate hybrid coordinates
    akFromFile.resize(nab);
    bkFromFile.resize(nab);

    if (fields.geometry()->getComm().rank() == 0) {
      size_t iVar2D = 0;
      for (const auto & var : varsToRead) {
        auto varField = globalData[var.name()];
        auto varView = atlas::array::make_view<double, 2>(varField);
        for (int jlevel = 0; jlevel < var.getLevels(); ++jlevel) {
          // Read data
          std::vector<double> zvar(ny*nx);
          if ((retval = nc_get_var_double(ncid, var_id[iVar2D], zvar.data()))) {
            ERR(retval, var2DName[iVar2D]);
          }
          ++iVar2D;

          // Copy data
          for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < grid.nx(j); ++i) {
              atlas::gidx_t gidx = grid.index(i, j);
              varView(gidx, jlevel) = zvar[j*nx+i];
            }
          }
        }
      }

      // Read data
      if ((retval = nc_get_var_double(ncid, ak_id, akFromFile.data())))
        ERR(retval, "hybrid_coef_A");
      if ((retval = nc_get_var_double(ncid, bk_id, bkFromFile.data())))
        ERR(retval, "hybrid_coef_B");

      // Close file
      if ((retval = nc_close(ncid))) ERR(retval, filePath);
    }

    // Broadcast hybrid coordinates
    fields.geometry()->getComm().broadcast(akFromFile.begin(), akFromFile.end(), 0);
    fields.geometry()->getComm().broadcast(bkFromFile.begin(), bkFromFile.end(), 0);
  } else if (ioFormat_ == "arome fa") {
#ifdef READFA
    if (!transSetup) {
      // Get cell sizes
      const double startx = xspec.getDouble("start");
      const double endx = xspec.getDouble("end");
      const double dx = (endx-startx)/static_cast<double>(nx-1);
      const double starty = yspec.getDouble("start");
      const double endy = yspec.getDouble("end");
      const double dy = (endy-starty)/static_cast<double>(ny-1);

      // Configure transform
      trans_use_mpi(true);
      trans_set_leq_regions(false);
      const int nprgpew = std::min(1,
        static_cast<int>(std::sqrt(static_cast<double>(fields.geometry()->getComm().size()))));
      trans_set_nprgpew(nprgpew);

      // Setup transform structure
      trans_new(&trans);
      trans_set_resol_lam(&trans, nx, ny, dx, dy);
      trans_set_trunc_lam(&trans, (nx-1)/2, (ny-1)/2);
      trans_setup(&trans);
      transSetup = true;
    }

    // Update configuration
    eckit::LocalConfiguration updatedConfig(conf);
    updatedConfig.set("filepath", filePath);
    updatedConfig.set("nvar2d", nVar2D);
    updatedConfig.set("prefix vector", preVec);
    updatedConfig.set("level vector", levVec);
    updatedConfig.set("variable vector", varVec);

    // Create hybrid coordinates fieldset
    atlas::FieldSet akbkData;

    // Read FA file
    fieldsio_arome_fa_read_f90(updatedConfig, &fields.geometry()->getComm(), fs.get(), &trans,
      akbkData.get(), globalData.get());

    // Get hybrid coordinates dimension
    if (fields.geometry()->getComm().rank() == 0) {
      nab = akbkData["ak"].shape(0);
    }

    // Broadcast hybrid coordinates dimension
    fields.geometry()->getComm().broadcast(nab, 0);

    // Allocate hybrid coordinates
    akFromFile.resize(nab);
    bkFromFile.resize(nab);

    // Get hybrid coordinates
    if (fields.geometry()->getComm().rank() == 0) {
      // Get ak/bk views
      const auto akView = atlas::array::make_view<double, 1>(akbkData["ak"]);
      const auto bkView = atlas::array::make_view<double, 1>(akbkData["bk"]);

      // Copy ak/bk
      for (size_t jab = 0; jab < nab; ++jab) {
        akFromFile[jab] = akView(jab);
        bkFromFile[jab] = bkView(jab);
      }
    }

    // Broadcast hybrid coordinates
    fields.geometry()->getComm().broadcast(akFromFile.begin(), akFromFile.end(), 0);
    fields.geometry()->getComm().broadcast(bkFromFile.begin(), bkFromFile.end(), 0);
#else
    // Format not available
    throw eckit::Exception("arome fa format not available", Here());
#endif
  }

  // Scatter data from main processor
  fs.scatter(globalData, fsetToRead);

  // Processing of derived variables
  for (const auto & var : vars) {
    if (var.name() == "air_pressure_at_surface"
      || var.name() == "air_pressure" || var.name() == "air_pressure_at_half_levels") {
      // Create field
      atlas::Field varField = fs.createField<double>(atlas::option::name(var.name())
        | atlas::option::levels(var.getLevels()));
      fields.fieldSet().add(varField);

      // Get view
      auto varView = atlas::array::make_view<double, 2>(varField);

      // Get read view
      const auto logOfPsView = atlas::array::make_view<double, 2>(fsetToRead["SURFPRESSION"]);

      if (var.name() == "air_pressure_at_surface") {
        // Apply exp
        for (int jnode = 0; jnode < varField.shape(0); ++jnode) {
          varView(jnode, 0) = std::exp(logOfPsView(jnode, 0));
        }
      } else if (var.name() == "air_pressure" || var.name() == "air_pressure_at_half_levels") {
        // Retrieve air_pressure or air_pressure_at_half_levels from ak/bk

        // Hybrid coordinates
        std::vector<double> ak(var.getLevels());
        std::vector<double> bk(var.getLevels());

        if (var.name() == "air_pressure") {
          // Pressure at full levels
          ASSERT(static_cast<int>(nab) == var.getLevels()+1);
          for (int jlevel = 0; jlevel < var.getLevels(); ++jlevel) {
            ak[jlevel] = 0.5*(akFromFile[jlevel]+akFromFile[jlevel+1]);
            bk[jlevel] = 0.5*(bkFromFile[jlevel]+bkFromFile[jlevel+1]);
          }
        } else if (var.name() == "air_pressure_at_half_levels") {
          // Pressure at half levels
          ASSERT(static_cast<int>(nab) == var.getLevels());
          for (int jlevel = 0; jlevel < var.getLevels(); ++jlevel) {
            ak[jlevel] = akFromFile[jlevel];
            bk[jlevel] = bkFromFile[jlevel];
          }
        }

        // Compute pressure
        for (int jnode = 0; jnode < varField.shape(0); ++jnode) {
          for (int jlevel = 0; jlevel < var.getLevels(); ++jlevel) {
            varView(jnode, jlevel) = ak[jlevel] + bk[jlevel]*std::exp(logOfPsView(jnode, 0));
          }
        }
      }
    }

    if (var.name() == "height_above_mean_sea_level_at_surface") {
      // Create field
      atlas::Field varField = fs.createField<double>(atlas::option::name(var.name())
        | atlas::option::levels(var.getLevels()));
      fields.fieldSet().add(varField);

      // Get views
      const auto zsView = atlas::array::make_view<double, 2>(fsetToRead["SPECSURFGEOPOTEN"]);
      auto varView = atlas::array::make_view<double, 2>(varField);

      // Compute surface height
      const double gInv = 1.0/9.81;
      for (int jnode = 0; jnode < varField.shape(0); ++jnode) {
        varView(jnode, 0) = zsView(jnode, 0)*gInv;
      }
    }

    if (var.name() == "eastward_wind" || var.name() == "northward_wind") {
      // Compute spherical winds

      // Create field
      atlas::Field varField = fs.createField<double>(atlas::option::name(var.name())
        | atlas::option::levels(var.getLevels()));
      fields.fieldSet().add(varField);

      // Get views
      const auto uView = atlas::array::make_view<double, 2>(fsetToRead["WIND.U.PHYS"]);
      const auto vView = atlas::array::make_view<double, 2>(fsetToRead["WIND.V.PHYS"]);
      auto varView = atlas::array::make_view<double, 2>(varField);

      // Get lon/lat view
      const auto lonlatView = atlas::array::make_view<double, 2>(fs.lonlat());

      for (int jnode = 0; jnode < varField.shape(0); ++jnode) {
        // Get local point
        atlas::PointLonLat p({lonlatView(jnode, 0), lonlatView(jnode, 1)});

        if (var.name() == "eastward_wind") {
          // Get local Jacobian
          double dx_dlon = fields.geometry()->grid().projection().jacobian(p).dx_dlon();
          double dy_dlon = fields.geometry()->grid().projection().jacobian(p).dy_dlon();

          // Normalize Jacobian
          const double dlonNorm = 1.0/std::sqrt(dx_dlon*dx_dlon+dy_dlon*dy_dlon);
          dx_dlon *= dlonNorm;
          dy_dlon *= dlonNorm;

          // Apply transform
          for (int jlevel = 0; jlevel < varField.shape(1); ++jlevel) {
            varView(jnode, jlevel) = uView(jnode, jlevel)*dx_dlon + vView(jnode, jlevel)*dy_dlon;
          }
        } else if (var.name() == "northward_wind") {
          // Get local Jacobian
          double dx_dlat = fields.geometry()->grid().projection().jacobian(p).dx_dlat();
          double dy_dlat = fields.geometry()->grid().projection().jacobian(p).dy_dlat();

          // Normalize Jacobian
          const double dlatNorm = 1.0/std::sqrt(dx_dlat*dx_dlat+dy_dlat*dy_dlat);
          dx_dlat *= dlatNorm;
          dy_dlat *= dlatNorm;

          // Apply transform
          for (int jlevel = 0; jlevel < varField.shape(1); ++jlevel) {
            varView(jnode, jlevel) = uView(jnode, jlevel)*dx_dlat + vView(jnode, jlevel)*dy_dlat;
          }
        }
      }
    }
  }

  // Processing of direct variables (share variables)
  for (const auto & var : vars) {
    if (var.name() == "geographical_x_wind") {
      fields.fieldSet().add(fsetToRead["WIND.U.PHYS"]);
      fields.fieldSet()["WIND.U.PHYS"].rename("geographical_x_wind");
    }

    if (var.name() == "geographical_y_wind") {
      fields.fieldSet().add(fsetToRead["WIND.V.PHYS"]);
      fields.fieldSet()["WIND.V.PHYS"].rename("geographical_y_wind");
    }

    if (var.name() == "air_temperature") {
      fields.fieldSet().add(fsetToRead["TEMPERATURE"]);
      fields.fieldSet()["TEMPERATURE"].rename("air_temperature");
    }

    if (var.name() == "log_of_air_pressure_at_surface") {
      fields.fieldSet().add(fsetToRead["SURFPRESSION"]);
      fields.fieldSet()["SURFPRESSION"].rename("log_of_air_pressure_at_surface");
    }

    if (var.name() == "water_vapor_mixing_ratio_wrt_moist_air") {
      fields.fieldSet().add(fsetToRead["HUMI.SPECIFI"]);
      fields.fieldSet()["HUMI.SPECIFI"].rename("water_vapor_mixing_ratio_wrt_moist_air");
    }
  }

  // Code is too complicated, mark dirty to be safe
  fields.fieldSet().set_dirty();

  oops::Log::trace() << classname() << "::read done" << std::endl;
}

// -----------------------------------------------------------------------------

void FieldsIOArome::write(const eckit::Configuration & conf,
                          const Fields & fields) const {
  oops::Log::trace() << classname() << "::write starting" << std::endl;

  // StructuredColumns
  atlas::functionspace::StructuredColumns fs(fields.geometry()->functionSpace());

  // Get grid
  atlas::StructuredGrid grid = fs.grid();

  // Get file path
  std::string filePath = conf.getString("filepath");
  if (conf.has("member")) {
    std::ostringstream out;
    out << std::setfill('0') << std::setw(6) << conf.getInt("member");
    filePath.append("_");
    filePath.append(out.str());
  }

  // NetCDF file path
  if (ioFormat_ == "arome netcdf") {
    filePath = filePath + ".nc";
  }

  // Define fields.fieldSet() to write
  atlas::FieldSet fsetToWrite;
  for (const auto & varName : fields.fieldSet().field_names()) {
    if (varName == "geographical_x_wind") {
      atlas::Field field = fields.fieldSet()[varName].clone();
      field.rename("WIND.U.PHYS");
      fsetToWrite.add(field);
    }

    if (varName == "geographical_y_wind") {
      atlas::Field field = fields.fieldSet()[varName].clone();
      field.rename("WIND.V.PHYS");
      fsetToWrite.add(field);
    }

    if (varName == "air_temperature") {
      atlas::Field field = fields.fieldSet()[varName].clone();
      field.rename("TEMPERATURE");
      fsetToWrite.add(field);
    }

    if (varName == "log_of_air_pressure_at_surface") {
      atlas::Field field = fields.fieldSet()[varName].clone();
      field.rename("SURFPRESSION");
      fsetToWrite.add(field);
    }

    if (varName == "water_vapor_mixing_ratio_wrt_moist_air") {
      atlas::Field field = fields.fieldSet()[varName].clone();
      field.rename("HUMI.SPECIFI");
      fsetToWrite.add(field);
    }
  }

  // Global data
  atlas::FieldSet globalData;
  for (const auto & field : fsetToWrite) {
    atlas::Field glbField = fs.createField<double>(atlas::option::name(field.name())
     | atlas::option::levels(field.levels()) | atlas::option::global());
    globalData.add(glbField);
  }

  // Gather data to main processor
  fs.gather(fsetToWrite, globalData);

  // File variables names
  size_t nVar2D = 0;
  std::vector<std::string> preVec;
  std::vector<int> levVec;
  std::vector<std::string> varVec;
  for (const auto & field : fsetToWrite) {
    for (int jlevel = 0; jlevel < field.levels(); ++jlevel) {
      if (field.name() == "SURFPRESSION") {
        preVec.push_back("SURF");
        levVec.push_back(0);
        varVec.push_back("PRESSION");
      } else {
        preVec.push_back("S");
        levVec.push_back(jlevel+1);
        varVec.push_back(field.name());
      }
      ++nVar2D;
    }
  }

  // Get sizes
  const atlas::util::Config xspec = grid.xspace().spec();
  const atlas::util::Config yspec = grid.yspace().spec();
  const size_t nx = xspec.getInt("N");
  const size_t ny = yspec.getInt("N");

  oops::Log::info() << "Info     : Writing file: " << filePath << std::endl;

  if (ioFormat_ == "arome netcdf") {
    throw eckit::Exception("arome netcdf writer not implemented yet", Here());
  } else if (ioFormat_ == "arome fa") {
#ifdef READFA
    if (!transSetup) {
      // Get cell sizes
      const double startx = xspec.getDouble("start");
      const double endx = xspec.getDouble("end");
      const double dx = (endx-startx)/static_cast<double>(nx-1);
      const double starty = yspec.getDouble("start");
      const double endy = yspec.getDouble("end");
      const double dy = (endy-starty)/static_cast<double>(ny-1);

      // Configure transform
      trans_use_mpi(true);
      trans_set_leq_regions(false);
      const int nprgpew = std::min(1,
        static_cast<int>(std::sqrt(static_cast<double>(fields.geometry()->getComm().size()))));
      trans_set_nprgpew(nprgpew);

      // Setup transform structure
      trans_new(&trans);
      trans_set_resol_lam(&trans, nx, ny, dx, dy);
      trans_set_trunc_lam(&trans, (nx-1)/2, (ny-1)/2);
      trans_setup(&trans);
      transSetup = true;
    }

    // Update configuration
    eckit::LocalConfiguration updatedConfig(conf);
    updatedConfig.set("filepath", filePath);
    updatedConfig.set("nvar2d", nVar2D);
    updatedConfig.set("prefix vector", preVec);
    updatedConfig.set("level vector", levVec);
    updatedConfig.set("variable vector", varVec);

    // Copy existing FA file
    if (fields.geometry()->getComm().rank() == 0) {
      const std::string originFilePath = conf.getString("origin filepath");
      std::ifstream src(originFilePath, std::ios::binary);
      std::ofstream dst(filePath, std::ios::binary);
      dst << src.rdbuf() << std::flush;
    }
    fields.geometry()->getComm().barrier();

    // Write FA file
    fieldsio_arome_fa_write_f90(updatedConfig, &fields.geometry()->getComm(), fs.get(), &trans,
      globalData.get());
#else
    // Format not available
    throw eckit::Exception("arome fa format not available", Here());
#endif
  }

  oops::Log::trace() << classname() << "::write done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
