# (C) Copyright 2025 Meteorlogisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Create the test input / reference / data directories
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testref )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testdata )

# Link ufo_variable_name_map.yaml
link_file( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} testinput/ufo_variable_name_map.yaml )

# Add tests
create_test( monglob geometry 8 test_geometry )
create_test( monglob convertstate 8 convertstate )
create_test( monglob convertstate_levsel 8 convertstate )

if( ENABLE_OBS )
    create_test( monglob hofx3d_air_pressure 8 hofx3d )
endif()
  
if( ENABLE_SABER )
    create_test( monglob dirac_3d 8 error_covariance_toolbox )
endif()
