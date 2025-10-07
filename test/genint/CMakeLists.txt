# (C) Copyright 2025 Meteorlogisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Create the test input / reference / data directories
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testref )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testdata )

# List test data
list( APPEND genint_data

testdata/geos_cf_N10_subset.20210801_0000z.nc
testdata/camchem_N30x20_subset.nc
testdata/wrfout_d01_subset_20230519_000000.nc
testdata/tropomi_no2_tropo_2020090318_m.nc4
testdata/tropomi_no2_tropo_2020090318_m_singleobs.nc4

)

# Link test data
link_files( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${genint_data} )

# Add tests
create_test( genint geometry_regular_lonlat 6 test_geometry )
create_test( genint geometry_lambertCC 6 test_geometry )
create_test( genint convertstate_regular_lonlat 6 convertstate )
create_test( genint convertstate_nonuni_lonlat 6 convertstate )
create_test( genint convertstate_varcha_regular_lonlat 6 convertstate )
create_test( genint convertstate_varcha_nonuni_lonlat 6 convertstate )

if( ENABLE_OBS )
    create_test( genint hofx3d_regular_lonlat 6 hofx3d )
    create_test( genint hofx3d_nonuni_lonlat 6 hofx3d )
    create_test( genint hofx3d_lambertCC 6 hofx3d )
endif()
