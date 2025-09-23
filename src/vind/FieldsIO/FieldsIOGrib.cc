/*
 * (C) Copyright 2025 Meteorologisk Institutt
 *
 */

#include "vind/FieldsIO/FieldsIOGrib.h"

#include <eccodes.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <string>
#include <vector>

#include "atlas/functionspace.h"

#include "eckit/config/Configuration.h"
#include "eckit/exception/Exceptions.h"

#include "oops/base/Variables.h"
#include "oops/util/Logger.h"

#include "vind/Fields.h"
#include "vind/Geometry.h"

namespace vind {

// -----------------------------------------------------------------------------

static FieldsIOMaker<FieldsIOGrib> makerGrib_("grib");

// -----------------------------------------------------------------------------

void FieldsIOGrib::read(const oops::Variables & vars,
                        const eckit::Configuration & conf,
                        Fields & fields) const {
  oops::Log::trace() << classname() << "::read starting" << std::endl;

  // Get geometry
  const Geometry & geom(fields.geometry());

  // Build file path
  std::string filePath = conf.getString("filepath");
  if (conf.has("member")) {
    std::ostringstream out;
    out << std::setfill('0') << std::setw(6) << conf.getInt("member");
    filePath.append("_");
    filePath.append(out.str());
  }

  // Grib file path
  filePath.append(".");
  filePath.append(conf.getString("grib extension", "grib2"));

  // Get levels
  std::vector<int> levels;
  if (!conf.get("levels", levels)) {
    int levelMax = 0;
    for (const auto & var : vars) {
      levelMax = std::max(levelMax, var.getLevels());
    }
    for (int jlevel = 0; jlevel < levelMax; ++jlevel) {
      levels.push_back(jlevel+1);
    }
  }

  // Clear local fieldset
  fields.fieldSet().clear();

  // Create local fieldset
  for (const auto & var : vars) {
    atlas::Field field = geom.functionSpace().createField<double>(
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
    atlas::Field field = geom.functionSpace().createField<double>(
      atlas::option::name(var.name())
      | atlas::option::levels(var.getLevels()) | atlas::option::global());
    globalData.add(field);
  }

  // Grib input
  if (geom.getComm().rank() == 0) {
    oops::Log::info() << "Info     : Reading file: " << filePath << std::endl;

    // Initialization
    int ret;
    codes_index* index;
    codes_handle* h;

    // Create index of file contents for cfVarName, typeOfLevel and level
    index = codes_index_new_from_file(0, filePath.c_str(), "cfVarName,typeOfLevel,level",
      &ret);
    CODES_CHECK(ret, 0);

    for (const auto & var : vars) {
      // Get field view
      auto varView = atlas::array::make_view<double, 2>(globalData[var.name()]);

      // Select variable and type of level
      CODES_CHECK(codes_index_select_string(index, "cfVarName", var.name().c_str()), 0);
      CODES_CHECK(codes_index_select_string(index, "typeOfLevel", "hybrid"), 0);

      for (int jlevel = 0; jlevel < var.getLevels(); ++jlevel) {
        // Select level
        CODES_CHECK(codes_index_select_long(index, "level", levels[jlevel]), 0);

        // Create handle
        h = codes_handle_new_from_index(index, &ret);
        CODES_CHECK(ret, 0);

        // Print all available keys
        codes_keys_iterator *kit = codes_keys_iterator_new(h, 0, NULL);
        while (codes_keys_iterator_next(kit) == 1) {
          oops::Log::debug() << "Key: " << codes_keys_iterator_get_name(kit) << std::endl;
        }

        // Get the data size
        size_t values_len = 0;
        CODES_CHECK(codes_get_size(h, "values", &values_len), 0);

        // Allocate data
        std::vector<double> values;
        values.resize(values_len);

        // Get data
        CODES_CHECK(codes_get_double_array(h, "values", values.data(), &values_len), 0);

        // Copy data to FieldSet
        for (size_t jnode = 0; jnode < values_len; ++jnode) {
          varView(jnode, jlevel) = values[jnode];
        }

        // Delete handle
        CODES_CHECK(codes_handle_delete(h), 0);
      }
    }

    // Check number of levels
    h = codes_handle_new_from_index(index, &ret);
    if (ret == 0) {
      throw eckit::Exception("Mismatch between level numbers in file and geometry", Here());
    }

    // Delete index
    codes_index_delete(index);
  }

  // Scatter data from main processor
  if (geom.functionSpace().type() == "StructuredColumns") {
    // StructuredColumns
    atlas::functionspace::StructuredColumns fs(geom.functionSpace());
    fs.scatter(globalData, fields.fieldSet());
  } else if (geom.functionSpace().type() == "NodeColumns") {
    // NodeColumns
    atlas::functionspace::NodeColumns fs(geom.functionSpace());
    fs.scatter(globalData, fields.fieldSet());
  }

  // Code is too complicated, mark dirty to be safe
  fields.fieldSet().set_dirty();

  oops::Log::trace() << classname() << "::read done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace vind
