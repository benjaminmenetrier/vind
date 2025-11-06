# (C) Copyright 2025 Meteorlogisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Create the test input / reference / data directories
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testref )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testdata )

# Link ufo_variable_name_map.yaml
link_file( ${CMAKE_CURRENT_SOURCE_DIR}/../monarch ${CMAKE_CURRENT_BINARY_DIR} testinput/ufo_variable_name_map.yaml )

# Print out data directory
message( STATUS "AQ_DATA_DIR: " $ENV{MONARCH_DATA_DIR} )

# List test data
list( APPEND aq_data

testdata/E0000
testdata/E0001
testdata/E0002
testdata/E0003
testdata/E0004
testdata/E0005
testdata/forecast
testdata/HDAT_AIRBASE_PM10-2022070112.nc
testdata/HDAT_AIRBASE_O3-2022070112.nc

)

# Link test data
link_files( $ENV{MONARCH_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${aq_data} )

# Add tests
math(EXPR 3DCORES "8 * ${VIND_TEST_CORES_FACTOR}")
math(EXPR 4DCORES "12 * ${VIND_TEST_CORES_FACTOR}")
math(EXPR LTCORES "2 * ${VIND_TEST_CORES_FACTOR}")

create_test( aq geometry ${3DCORES} test_geometry )
create_test( aq state ${3DCORES} test_state )
create_test( aq convertstate ${3DCORES} convertstate )
create_test( aq convertstate_logtransf ${3DCORES} convertstate )
create_test( aq increment ${3DCORES} test_increment )
create_test( aq ens_mean_variance ${3DCORES} ens_mean_variance )

if( ENABLE_OBS )
    create_test( aq hofx3d_constant ${3DCORES} hofx3d )
    create_test( aq hofx3d_logarithm ${3DCORES} hofx3d )
endif()
  
if( ENABLE_SABER )
    create_test( aq dirac_no_loc_3d ${3DCORES} error_covariance_toolbox )
    create_test( aq dirac_no_loc_3d_lt ${3DCORES} error_covariance_toolbox )
    create_test( aq dirac_no_loc_4d_id ${4DCORES} error_covariance_toolbox )
    create_test( aq dirac_no_loc_4d ${4DCORES} error_covariance_toolbox )
    create_test( aq dirac_3d ${3DCORES} error_covariance_toolbox )
    create_test( aq dirac_4d ${4DCORES} error_covariance_toolbox )
    create_test( aq dirac_4d_FL ${4DCORES} error_covariance_toolbox )
endif()

if ( ENABLE_SABER AND ENABLE_OBS )
    create_test( aq 3densvar ${3DCORES} variational )
    create_test( aq 3densvar_lt ${3DCORES} variational )
    create_test( aq localization_training ${LTCORES} error_covariance_toolbox )
    create_test( aq 4densvar ${4DCORES} variational )
    create_test( aq 4densvar_lt ${4DCORES} variational )
endif()
