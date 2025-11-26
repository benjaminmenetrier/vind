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

# Print data directory
message( STATUS "MONGLOB_DATA_DIR: " $ENV{MONARCH_DATA_DIR} )

# List test data
list( APPEND monglob_data

testdata/G000
testdata/G001
testdata/G002
testdata/G003
testdata/G004
testdata/G005
testdata/G006
testdata/HDAT_AIRBASE_PM10-GLO-2017121612.nc

)

# Link test data
link_files( $ENV{MONARCH_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${monglob_data} )

# Add tests
math(EXPR 3DCORES "8 * ${VIND_TEST_CORES_FACTOR}")
math(EXPR 4DCORES "12 * ${VIND_TEST_CORES_FACTOR}")

create_test( monglob geometry ${3DCORES} test_geometry )
create_test( monglob convertstate ${3DCORES} convertstate )
create_test( monglob convertstate_levsel ${3DCORES} convertstate )

if( ENABLE_OBS )
    create_test( monglob hofx3d_air_pressure ${3DCORES} hofx3d )
endif()
  
if( ENABLE_SABER )
    create_test( monglob dirac_3d ${3DCORES} error_covariance_toolbox )
endif()
