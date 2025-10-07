# (C) Copyright 2025 Meteorlogisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Create the test input / reference / data directories
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testref )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testdata )

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
testdata/HDAT_AIRBASE_PM10-GLO-2017121612.nc

)

# Link test data
link_files( $ENV{MONARCH_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${aq_data} )

# Add tests
create_test( aq geometry 8 test_geometry )
create_test( aq state 8 test_state )
create_test( aq convertstate 8 convertstate )
create_test( aq convertstate_logtransf 8 convertstate )
create_test( aq increment 8 test_increment )
create_test( aq ens_mean_variance 8 ens_mean_variance )

if( ENABLE_OBS )
    create_test( aq hofx3d_constant 8 hofx3d )
    create_test( aq hofx3d_logarithm 8 hofx3d )
endif()
  
if( ENABLE_SABER )
    create_test( aq dirac_no_loc_3d 8 error_covariance_toolbox )
    create_test( aq dirac_no_loc_3d_lt 8 error_covariance_toolbox )
    create_test( aq dirac_no_loc_4d_id 12 error_covariance_toolbox )
    create_test( aq dirac_no_loc_4d 12 error_covariance_toolbox )
    create_test( aq dirac_3d 8 error_covariance_toolbox )
    create_test( aq dirac_4d 12 error_covariance_toolbox )
    create_test( aq dirac_4d_FL 12 error_covariance_toolbox )
endif()

if ( ENABLE_SABER AND ENABLE_OBS )
    create_test( aq 3densvar 8 variational )
    create_test( aq 3densvar_lt 8 variational )
    create_test( aq 4densvar 12 variational )
    create_test( aq 4densvar_lt 12 variational )
endif()
