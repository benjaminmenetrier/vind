/*
 * (C) Copyright 2025 Meteorologisk Institutt
 * 
 */

#include "vind/Utilities.h"

#include "atlas/array.h"

#include "eckit/exception/Exceptions.h"

#include "oops/util/missingValues.h"

namespace vind {

// -----------------------------------------------------------------------------

void copyFieldSetWithoutFunctionSpace(const atlas::FieldSet & otherFset,
                                      atlas::FieldSet & fset) {
  fset.clear();
  for (const auto & otherField : otherFset) {
    // Check whether the input Field is associated with a FunctionSpace
    atlas::Field field;

    // Create Field without FunctionSpace
    field = atlas::Field(otherField.name(), atlas::array::make_datatype<double>(),
      atlas::array::make_shape(otherField.shape(0), otherField.shape(1)));

    // Copy data
    if (field.rank() == 2) {
      auto view = atlas::array::make_view<double, 2>(field);
      auto otherView = atlas::array::make_view<double, 2>(otherField);
      for (atlas::idx_t jnode = 0; jnode < otherField.shape(0); ++jnode) {
        for (atlas::idx_t jlevel = 0; jlevel < otherField.shape(1); ++jlevel) {
            view(jnode, jlevel) = otherView(jnode, jlevel);
        }
      }
    } else {
      throw eckit::Exception("copyFieldSetWithoutFunctionSpace: wrong rank", Here());
    }

    // Copy metadata
    field.metadata() = otherField.metadata();

    // Add field
    fset.add(field);
  }

  // Copy fieldset name
  fset.name() = otherFset.name();
}

// -----------------------------------------------------------------------------

double dotProductFieldSetsWithoutFunctionSpace(const atlas::FieldSet & fset1,
                                               const atlas::FieldSet & fset2,
                                               const std::vector<std::string> & vars,
                                               const eckit::mpi::Comm & comm) {
  // Compute dot product
  double dp = 0.0;

  for (const auto & var : vars) {
    // Check fields presence
    if (fset1.has(var) && fset2.has(var)) {
      // Get fields
      atlas::Field field1 = fset1[var];
      atlas::Field field2 = fset2[var];

      // Compute dot product
      if (field1.rank() == 2 && field2.rank() == 2) {
        // Check fields consistency
        ASSERT(field1.shape(0) == field2.shape(0));
        ASSERT(field1.shape(1) == field2.shape(1));

        // Add contributions
        auto view1 = atlas::array::make_view<double, 2>(field1);
        auto view2 = atlas::array::make_view<double, 2>(field2);
        for (atlas::idx_t jnode = 0; jnode < field1.shape(0); ++jnode) {
          for (atlas::idx_t jlevel = 0; jlevel < field1.shape(1); ++jlevel) {
            if (view1(jnode, jlevel) != util::missingValue<double>()
              && view2(jnode, jlevel) != util::missingValue<double>()) {
              dp += view1(jnode, jlevel)*view2(jnode, jlevel);
            }
          }
        }
      } else {
        throw eckit::Exception("dotProductFieldsWithoutFunctionSpaceLocal: wrong rank", Here());
      }
    }
  }

  // Allreduce
  comm.allReduceInPlace(dp, eckit::mpi::sum());

  // Return dot product
  return dp;
}

// -----------------------------------------------------------------------------

}  // namespace vind
