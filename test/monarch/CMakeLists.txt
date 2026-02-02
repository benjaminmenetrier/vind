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

# Print data directory
message( STATUS "MONARCH_DATA_DIR: " $ENV{MONARCH_DATA_DIR} )

# List test data
list( APPEND data

testdata/000
testdata/001
testdata/002
testdata/003
testdata/004
testdata/005
testdata/006
testdata/HDAT_AIRBASE_PM10-2022070112.nc
testdata/HDAT_AIRBASE_O3-2022070112.nc

)

# Link test data
link_files( $ENV{MONARCH_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${data} )

# Add tests
math(EXPR 3DCORES "8 * ${VIND_TEST_CORES_FACTOR}")
math(EXPR 4DCORES "12 * ${VIND_TEST_CORES_FACTOR}")

create_test( monarch geometry_rotated ${3DCORES} test_geometry )
create_test( monarch state ${3DCORES} test_state )
create_test( monarch convertstate ${3DCORES} convertstate )
create_test( monarch convertstate_levsel ${3DCORES} convertstate )
create_test( monarch convertstate_levsel_scaling ${3DCORES} convertstate )
create_test( monarch convertstate_levsel_logtransf ${3DCORES} convertstate )
create_test( monarch increment ${3DCORES} test_increment )
create_test( monarch ens_mean_variance ${3DCORES} ens_mean_variance )

if( ENABLE_OBS )
    create_test( monarch hofx3d_constant ${3DCORES} hofx3d )
    create_test( monarch hofx3d_air_pressure ${3DCORES} hofx3d )
    create_test( monarch hofx3d_logarithm ${3DCORES} hofx3d )
endif()
  
if( ENABLE_SABER )
    create_test( monarch dirac_no_loc_3d ${3DCORES} error_covariance_toolbox )
    create_test( monarch dirac_no_loc_3d_lt ${3DCORES} error_covariance_toolbox )
    create_test( monarch dirac_no_loc_4d_id ${4DCORES} error_covariance_toolbox )
    create_test( monarch dirac_no_loc_4d ${4DCORES} error_covariance_toolbox )
    create_test( monarch dirac_3d ${3DCORES} error_covariance_toolbox )
    create_test( monarch dirac_3d_FL ${3DCORES} error_covariance_toolbox )
    create_test( monarch dirac_4d ${4DCORES} error_covariance_toolbox )
    create_test( monarch dirac_4d_FL ${4DCORES} error_covariance_toolbox )
endif()

if ( ENABLE_SABER AND ENABLE_OBS )
    create_test( monarch 3densvar ${3DCORES} variational )
    create_test( monarch 3densvar_lt ${3DCORES} variational )
    create_test( monarch 4densvar ${4DCORES} variational )
    create_test( monarch 4densvar_lt ${4DCORES} variational )
    create_test( monarch letkf_nonlinear_4d ${3DCORES} letkf )
endif()
