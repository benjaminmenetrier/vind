# (C) Copyright 2024 Meteorlogisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Macro to link a file
macro( link_file src_dir dest_dir )
    execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink ${src_dir}/${ARGN} ${dest_dir}/${ARGN} )
endmacro()

# Macro to link files
macro( link_files src_dir dest_dir )
    foreach( _f IN ITEMS ${ARGN} )
        execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink ${src_dir}/${_f} ${dest_dir}/${_f} )
    endforeach()
endmacro()

# Macro to create test
macro( create_test name mpi exec )
    # Link input file
    link_file( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} testinput/${name}.yaml )

    # Link reference file
    link_file( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} testref/${name}.ref )

    # Add test
    ecbuild_add_test( TARGET   vind_test_${name}_${mpi}
                      MPI      ${mpi}
                      COMMAND  vind_${exec}.x
                      ARGS     testinput/${name}.yaml
                      DEPENDS  vind_${exec}.x )
endmacro()

# Create the test input / reference / data directories
# ----------------------------------------------------
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testref )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testdata )

# Link test data
# --------------
if( ENABLE_GENINT_TESTS )
    list( APPEND genint_data

    testdata/geos_cf_N10_subset.20210801_0000z.nc
    testdata/camchem_N30x20_subset.nc
    testdata/wrfout_d01_subset_20230519_000000.nc
    testdata/tropomi_no2_tropo_2020090318_m.nc4
    testdata/tropomi_no2_tropo_2020090318_m_singleobs.nc4

    )

    link_files( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${genint_data} )
endif()

if( ENABLE_MONARCH_TESTS OR ENABLE_AQ_TESTS )
    message( STATUS "MONARCH_DATA_DIR: " $ENV{MONARCH_DATA_DIR} )
endif()

if( ENABLE_MONARCH_TESTS )
    list( APPEND monarch_data

    testdata/000
    testdata/001
    testdata/002
    testdata/003
    testdata/004
    testdata/005
    testdata/006
    testdata/HDAT_AIRBASE_PM10-2022070112.nc
    testdata/HDAT_AIRBASE_O3-2022070112.nc
    testdata/G000
    testdata/G001
    testdata/G002
    testdata/G003
    testdata/G004
    testdata/G005
    testdata/G006
    
    )

    link_files( $ENV{MONARCH_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${monarch_data} )
endif()

if( ENABLE_AQ_TESTS )
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

    link_files( $ENV{MONARCH_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${aq_data} )
endif()

# Tests parameters
# ----------------

if( ENABLE_GLB_TESTS )
    list( APPEND domain_list "glb" )
endif()
if( ENABLE_REG_TESTS )
    list( APPEND domain_list "reg" )
endif()
list( APPEND mpi_list "1" "4" )
list( APPEND hh_list "06" "12" "18" )

# Tests
# -----

foreach( mpi ${mpi_list} )
    # GLOBAL and REGIONAL tests
    foreach( domain ${domain_list} )
        # Attractor
        create_test( ${domain}_attractor ${mpi} forecast )

        # Interface tests
        create_test( ${domain}_test_geometry ${mpi} test_geometry )
        create_test( ${domain}_test_increment ${mpi} test_increment )
        create_test( ${domain}_test_model ${mpi} test_model )

        # Truth
        create_test( ${domain}_truth ${mpi} forecast )

        # Standard-deviation
        create_test( ${domain}_stddev ${mpi} convertstate )

        if( ENABLE_SABER )
            # Background
            create_test( ${domain}_background ${mpi} error_covariance_toolbox )

            # Ensemble
            create_test( ${domain}_ensemble ${mpi} error_covariance_toolbox )
        endif()

        if( ENABLE_OBS )
            # Make observations, 3d and 4d
            create_test( ${domain}_makeobs_3d ${mpi} hofx3d )
            create_test( ${domain}_makeobs_4d ${mpi} hofx4d )
        endif()

        if( ENABLE_SABER AND ENABLE_OBS )
            # 3DVar
            create_test( ${domain}_3dvar ${mpi} variational )

            # 4DVar
            create_test( ${domain}_4dvar ${mpi} variational )

            # 3DEnVar
            create_test( ${domain}_3densvar ${mpi} variational )

            # 4DEnVar
            math( EXPR mpi_par "${mpi}*3" )
            create_test( ${domain}_4densvar ${mpi} variational )
            create_test( ${domain}_4densvar_parallel ${mpi_par} variational )

            # LETKF
            create_test( ${domain}_getkf_nonlinear ${mpi} letkf )
            create_test( ${domain}_letkf_linear ${mpi} letkf )
            create_test( ${domain}_letkf_linear_4d ${mpi} letkf )
            create_test( ${domain}_letkf_nonlinear ${mpi} letkf )
            create_test( ${domain}_letkf_nonlinear_4d ${mpi} letkf )
            create_test( ${domain}_letkf_read_members ${mpi} letkf )
         endif()
    endforeach()
endforeach()

if( ENABLE_PYDDL95_TESTS )
    # PYDDL95 tests
    foreach( mpi ${mpi_list} )
        create_test( pyddl95_attractor ${mpi} forecast )
        create_test( pyddl95_truth ${mpi} forecast )
    endforeach()
endif()

if( ENABLE_GENINT_TESTS )
    # GENINT tests
    create_test( genint_geometry_regular_lonlat 6 test_geometry )
    create_test( genint_geometry_lambertCC 6 test_geometry )
    create_test( genint_convertstate_regular_lonlat 6 convertstate )
    create_test( genint_convertstate_nonuni_lonlat 6 convertstate )
    create_test( genint_convertstate_varcha_regular_lonlat 6 convertstate )
    create_test( genint_convertstate_varcha_nonuni_lonlat 6 convertstate )

    if( ENABLE_OBS )
        create_test( genint_hofx3d_regular_lonlat 6 hofx3d )
        create_test( genint_hofx3d_nonuni_lonlat 6 hofx3d )
        create_test( genint_hofx3d_lambertCC 6 hofx3d )
    endif()
endif()

if( ENABLE_MONARCH_TESTS )
    # MONARCH BSC tests
    create_test( monarch_geometry_rotated 8 test_geometry )
    create_test( monarch_state 8 test_state )
    create_test( monarch_convertstate 8 convertstate )
    create_test( monarch_convertstate_levsel 8 convertstate )
    create_test( monarch_convertstate_levsel_scaling 8 convertstate )
    create_test( monarch_convertstate_levsel_logtransf 8 convertstate )
    create_test( monarch_increment 8 test_increment )
    create_test( monarch_ens_mean_variance 8 ens_mean_variance )

    if( ENABLE_OBS )
        create_test( monarch_hofx3d_constant 8 hofx3d )
        create_test( monarch_hofx3d_air_pressure 8 hofx3d )
        create_test( monarch_hofx3d_logarithm 8 hofx3d )
    endif()
      
    if( ENABLE_SABER )
        create_test( monarch_dirac_no_loc_3d 8 error_covariance_toolbox )
        create_test( monarch_dirac_no_loc_3d_lt 8 error_covariance_toolbox )
        create_test( monarch_dirac_no_loc_4d_id 12 error_covariance_toolbox )
        create_test( monarch_dirac_no_loc_4d 12 error_covariance_toolbox )
        create_test( monarch_dirac_3d 8 error_covariance_toolbox )
        create_test( monarch_dirac_3d_FL 8 error_covariance_toolbox )
        create_test( monarch_dirac_4d 12 error_covariance_toolbox )
        create_test( monarch_dirac_4d_FL 12 error_covariance_toolbox )
    endif()

    if ( ENABLE_SABER AND ENABLE_OBS )
        create_test( monarch_3densvar 8 variational )
        create_test( monarch_3densvar_lt 8 variational )
        create_test( monarch_4densvar 12 variational )
        create_test( monarch_4densvar_lt 12 variational )
    endif()

    # MONARCH global BSC tests
    create_test( monglob_geometry 8 test_geometry )
    create_test( monglob_convertstate 8 convertstate )
    create_test( monglob_convertstate_levsel 8 convertstate )

    if( ENABLE_OBS )
        create_test( monglob_hofx3d_air_pressure 8 hofx3d )
    endif()
      
    if( ENABLE_SABER )
        create_test( monglob_dirac_3d 8 error_covariance_toolbox )
    endif()
    
endif()

if( ENABLE_AQ_TESTS )
    # AQ BSC tests
    create_test( aq_geometry 8 test_geometry )
    create_test( aq_state 8 test_state )
    create_test( aq_convertstate 8 convertstate )
    create_test( aq_convertstate_logtransf 8 convertstate )
    create_test( aq_increment 8 test_increment )
    create_test( aq_ens_mean_variance 8 ens_mean_variance )
    
    if( ENABLE_OBS )
        create_test( aq_hofx3d_constant 8 hofx3d )
        create_test( aq_hofx3d_logarithm 8 hofx3d )
    endif()
      
    if( ENABLE_SABER )
        create_test( aq_dirac_no_loc_3d 8 error_covariance_toolbox )
        create_test( aq_dirac_no_loc_3d_lt 8 error_covariance_toolbox )
        create_test( aq_dirac_no_loc_4d_id 12 error_covariance_toolbox )
        create_test( aq_dirac_no_loc_4d 12 error_covariance_toolbox )
        create_test( aq_dirac_3d 8 error_covariance_toolbox )
        create_test( aq_dirac_4d 12 error_covariance_toolbox )
        create_test( aq_dirac_4d_FL 12 error_covariance_toolbox )
    endif()

    if ( ENABLE_SABER AND ENABLE_OBS )
        create_test( aq_3densvar 8 variational )
        create_test( aq_3densvar_lt 8 variational )
        create_test( aq_4densvar 12 variational )
        create_test( aq_4densvar_lt 12 variational )
    endif()
endif()
