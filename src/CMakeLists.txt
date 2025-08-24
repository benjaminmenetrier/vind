# (C) Copyright 2024 Meteorologisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

add_subdirectory( vind )
add_subdirectory( mains )

ecbuild_add_test( TARGET vind_coding_norms_src
                  TYPE SCRIPT
                  COMMAND ${CMAKE_BINARY_DIR}/bin/vind_cpplint.py
                  ARGS --quiet --recursive ${CMAKE_CURRENT_SOURCE_DIR}
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
