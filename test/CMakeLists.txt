# (C) Copyright 2024 Meteorlogisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Macro to link files
macro( link_files src_dir dest_dir )
    foreach( _f IN ITEMS ${ARGN} )
        execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink ${src_dir}/${_f} ${dest_dir}/${_f} )
    endforeach()
endmacro()

# Macro to create test
macro( create_test name mpi exec )
    ecbuild_add_test( TARGET   vind_test_${name}_${mpi}
                      MPI      ${mpi}
                      COMMAND  vind_${exec}.x
                      ARGS     testinput/${name}.yaml
                      DEPENDS  vind_${exec}.x )
endmacro()

# Create the testdata directory
# -----------------------------
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

# Create the input/output directories
# -----------------------------------
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testref )

# Test input scripts
# ------------------
list( APPEND vind_testinput

# GLOBAL
testinput/3dvar.yaml
testinput/3densvar.yaml
testinput/4densvar.yaml
testinput/4densvar_parallel.yaml
testinput/background_06.yaml
testinput/background_12.yaml
testinput/background_18.yaml
testinput/ensemble_06.yaml
testinput/ensemble_12.yaml
testinput/ensemble_18.yaml
testinput/getkf_nonlinear.yaml
testinput/letkf_linear.yaml
testinput/letkf_linear_4d.yaml
testinput/letkf_nonlinear.yaml
testinput/letkf_nonlinear_4d.yaml
testinput/letkf_read_members.yaml
testinput/makeobs_06.yaml
testinput/makeobs_12.yaml
testinput/makeobs_18.yaml
testinput/stddev.yaml
testinput/truth_06.yaml
testinput/truth_12.yaml
testinput/truth_18.yaml

# REGIONAL
testinput/reg_3dvar.yaml
testinput/reg_3densvar.yaml
testinput/reg_4densvar.yaml
testinput/reg_4densvar_parallel.yaml
testinput/reg_background_06.yaml
testinput/reg_background_12.yaml
testinput/reg_background_18.yaml
testinput/reg_ensemble_06.yaml
testinput/reg_ensemble_12.yaml
testinput/reg_ensemble_18.yaml
testinput/reg_getkf_nonlinear.yaml
testinput/reg_letkf_linear.yaml
testinput/reg_letkf_linear_4d.yaml
testinput/reg_letkf_nonlinear.yaml
testinput/reg_letkf_nonlinear_4d.yaml
testinput/reg_letkf_read_members.yaml
testinput/reg_makeobs_06.yaml
testinput/reg_makeobs_12.yaml
testinput/reg_makeobs_18.yaml
testinput/reg_stddev.yaml
testinput/reg_truth_06.yaml
testinput/reg_truth_12.yaml
testinput/reg_truth_18.yaml

# PERSISTENCE
testinput/per_3dvar.yaml
testinput/per_background.yaml
testinput/per_makeobs.yaml
testinput/per_stddev.yaml
testinput/per_truth.yaml

# PYQG
testinput/pyqg_attractor.yaml
testinput/pyqg_truth.yaml

# GENINT
testinput/genint_convertstate_nonuni_lonlat.yaml
testinput/genint_convertstate_regular_lonlat.yaml
testinput/genint_convertstate_varcha_regular_lonlat.yaml
testinput/genint_convertstate_varcha_nonuni_lonlat.yaml
testinput/genint_geometry_lambertCC.yaml
testinput/genint_geometry_regular_lonlat.yaml
testinput/genint_hofx3d_lambertCC.yaml
testinput/genint_hofx3d_nonuni_lonlat.yaml
testinput/genint_hofx3d_regular_lonlat.yaml

)

list( APPEND monarch_testinput

# MONARCH REGIONAL
testinput/monarch_geometry_rotated.yaml
testinput/monarch_state.yaml
testinput/monarch_convertstate.yaml
testinput/monarch_convertstate_levsel.yaml
testinput/monarch_convertstate_levsel_scaling.yaml
testinput/monarch_convertstate_levsel_logtransf.yaml
testinput/monarch_increment.yaml
testinput/monarch_ens_mean_variance.yaml
testinput/monarch_ufo_variable_name_map.yaml
testinput/monarch_hofx3d_constant.yaml
testinput/monarch_hofx3d_air_pressure.yaml
testinput/monarch_hofx3d_logarithm.yaml
testinput/monarch_dirac_no_loc_3d.yaml
testinput/monarch_dirac_no_loc_3d_lt.yaml
testinput/monarch_dirac_no_loc_4d_id.yaml
testinput/monarch_dirac_no_loc_4d.yaml
testinput/monarch_dirac_3d.yaml
testinput/monarch_dirac_3d_FL.yaml
testinput/monarch_dirac_4d.yaml
testinput/monarch_dirac_4d_FL.yaml
testinput/monarch_3densvar.yaml
testinput/monarch_3densvar_lt.yaml
testinput/monarch_4densvar.yaml
testinput/monarch_4densvar_lt.yaml

# MONARCH GLOBAL
testinput/monglob_geometry.yaml
testinput/monglob_convertstate.yaml
testinput/monglob_convertstate_levsel.yaml
testinput/monglob_hofx3d_air_pressure.yaml
testinput/monglob_dirac_3d.yaml
)

list( APPEND aq_testinput

# MONARCH REGIONAL interpolated on CAMS regular grid
testinput/aq_geometry.yaml
testinput/aq_state.yaml
testinput/aq_convertstate.yaml
testinput/aq_convertstate_logtransf.yaml
testinput/aq_increment.yaml
testinput/aq_ens_mean_variance.yaml
testinput/monarch_ufo_variable_name_map.yaml
testinput/aq_hofx3d_constant.yaml
testinput/aq_hofx3d_logarithm.yaml
testinput/aq_dirac_no_loc_3d.yaml
testinput/aq_dirac_no_loc_3d_lt.yaml
testinput/aq_dirac_no_loc_4d_id.yaml
testinput/aq_dirac_no_loc_4d.yaml
testinput/aq_dirac_3d.yaml
testinput/aq_dirac_4d.yaml
testinput/aq_dirac_4d_FL.yaml
testinput/aq_3densvar.yaml
testinput/aq_3densvar_lt.yaml
testinput/aq_4densvar.yaml
testinput/aq_4densvar_lt.yaml

)

# Test output references
# ----------------------
list( APPEND vind_testref

# GLOBAL
testref/3dvar.ref
testref/3densvar.ref
testref/4densvar.ref
testref/4densvar_parallel.ref
testref/background_06.ref
testref/background_12.ref
testref/background_18.ref
testref/ensemble_06.ref
testref/ensemble_12.ref
testref/ensemble_18.ref
testref/getkf_nonlinear.ref
testref/letkf_linear.ref
testref/letkf_linear_4d.ref
testref/letkf_nonlinear.ref
testref/letkf_nonlinear_4d.ref
testref/letkf_read_members.ref
testref/makeobs_06.ref
testref/makeobs_12.ref
testref/makeobs_18.ref
testref/stddev.ref
testref/truth_06.ref
testref/truth_12.ref
testref/truth_18.ref

# REGIONAL
testref/reg_3dvar.ref
testref/reg_3densvar.ref
testref/reg_4densvar.ref
testref/reg_4densvar_parallel.ref
testref/reg_background_06.ref
testref/reg_background_12.ref
testref/reg_background_18.ref
testref/reg_ensemble_06.ref
testref/reg_ensemble_12.ref
testref/reg_ensemble_18.ref
testref/reg_getkf_nonlinear.ref
testref/reg_letkf_linear.ref
testref/reg_letkf_linear_4d.ref
testref/reg_letkf_nonlinear.ref
testref/reg_letkf_nonlinear_4d.ref
testref/reg_letkf_read_members.ref
testref/reg_makeobs_06.ref
testref/reg_makeobs_12.ref
testref/reg_makeobs_18.ref
testref/reg_stddev.ref
testref/reg_truth_06.ref
testref/reg_truth_12.ref
testref/reg_truth_18.ref

# PERSISTENCE
testref/per_3dvar.ref
testref/per_background.ref
testref/per_makeobs.ref
testref/per_stddev.ref
testref/per_truth.ref

# PYQG
testref/pyqg_attractor.ref
testref/pyqg_truth.ref

# GENINT
testref/genint_geometry_regular_lonlat.ref
testref/genint_geometry_lambertCC.ref
testref/genint_convertstate_regular_lonlat.ref
testref/genint_convertstate_nonuni_lonlat.ref
testref/genint_convertstate_varcha_regular_lonlat.ref
testref/genint_convertstate_varcha_nonuni_lonlat.ref
testref/genint_hofx3d_regular_lonlat.ref
testref/genint_hofx3d_nonuni_lonlat.ref
testref/genint_hofx3d_lambertCC.ref

)

list( APPEND monarch_testref

# MONARCH REGIONAL
testref/monarch_geometry_rotated.ref
testref/monarch_state.ref
testref/monarch_convertstate.ref
testref/monarch_convertstate_levsel.ref
testref/monarch_convertstate_levsel_scaling.ref
testref/monarch_convertstate_levsel_logtransf.ref
testref/monarch_increment.ref
testref/monarch_ens_mean_variance.ref
testref/monarch_hofx3d_constant.ref
testref/monarch_hofx3d_air_pressure.ref
testref/monarch_hofx3d_logarithm.ref
testref/monarch_dirac_no_loc_3d.ref
testref/monarch_dirac_no_loc_3d_lt.ref
testref/monarch_dirac_no_loc_4d_id.ref
testref/monarch_dirac_no_loc_4d.ref
testref/monarch_dirac_3d.ref
testref/monarch_dirac_3d_FL.ref
testref/monarch_dirac_4d.ref
testref/monarch_dirac_4d_FL.ref
testref/monarch_3densvar.ref
testref/monarch_3densvar_lt.ref
testref/monarch_4densvar.ref
testref/monarch_4densvar_lt.ref

# MONARCH GLOBAL
testref/monglob_geometry.ref
testref/monglob_convertstate.ref
testref/monglob_convertstate_levsel.ref
testref/monglob_hofx3d_air_pressure.ref
testref/monglob_dirac_3d.ref
)

list( APPEND aq_testref

# MONARCH REGIONAL interpolated on CAMS regular grid
testref/aq_geometry.ref
testref/aq_state.ref
testref/aq_convertstate.ref
testref/aq_convertstate_logtransf.ref
testref/aq_increment.ref
testref/aq_ens_mean_variance.ref
testref/aq_hofx3d_constant.ref
testref/aq_hofx3d_logarithm.ref
testref/aq_dirac_no_loc_3d.ref
testref/aq_dirac_no_loc_3d_lt.ref
testref/aq_dirac_no_loc_4d_id.ref
testref/aq_dirac_no_loc_4d.ref
testref/aq_dirac_3d.ref
testref/aq_dirac_4d.ref
testref/aq_dirac_4d_FL.ref
testref/aq_3densvar.ref
testref/aq_3densvar_lt.ref
testref/aq_4densvar.ref
testref/aq_4densvar_lt.ref

)

# Link input configurations and output references
# -----------------------------------------------
link_files( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${vind_testinput} )
link_files( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${vind_testref} )
if( ENABLE_MONARCH_TESTS )
    link_files( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${monarch_testinput} )
    link_files( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${monarch_testref} )
endif()
if( ENABLE_AQ_TESTS )
    link_files( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${aq_testinput} )
    link_files( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${aq_testref} )
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
        create_test( ${domain}_stddev ${mpi} convertstate )

        foreach( hh ${hh_list} )
            create_test( ${domain}_truth_${hh} ${mpi} convertstate )

            if( ENABLE_SABER )
                create_test( ${domain}_background_${hh} ${mpi} error_covariance_toolbox )
                create_test( ${domain}_ensemble_${hh} ${mpi} error_covariance_toolbox )
            endif()

            if( ENABLE_OBS )
                create_test( ${domain}_makeobs_${hh} ${mpi} hofx3d )
            endif()
        endforeach()

        if( ENABLE_SABER AND ENABLE_OBS )
            create_test( ${domain}_3dvar ${mpi} variational )
            create_test( ${domain}_3densvar ${mpi} variational )
            create_test( ${domain}_4densvar ${mpi} variational )

            math( EXPR mpi_par "${mpi}*3" )
            create_test( ${domain}_4densvar_parallel ${mpi_par} variational )
            create_test( ${domain}_getkf_nonlinear ${mpi} letkf )
            create_test( ${domain}_letkf_linear ${mpi} letkf )
            create_test( ${domain}_letkf_linear_4d ${mpi} letkf )
            create_test( ${domain}_letkf_nonlinear ${mpi} letkf )
            create_test( ${domain}_letkf_nonlinear_4d ${mpi} letkf )
            create_test( ${domain}_letkf_read_members ${mpi} letkf )
         endif()
    endforeach()
endforeach()

if( ENABLE_PER_TESTS )
    # PERSISTENCE tests
    foreach( mpi ${mpi_list} )
        create_test( per_stddev ${mpi} convertstate )
        create_test( per_truth ${mpi} forecast )
        create_test( per_background ${mpi} forecast )
        create_test( per_makeobs ${mpi} hofx4d )
        create_test( per_3dvar ${mpi} variational )
    endforeach()
endif()

if( ENABLE_PYQG_TESTS )
    # PERSISTENCE tests
    foreach( mpi ${mpi_list} )
        create_test( pyqg_attractor ${mpi} forecast )
        create_test( pyqg_truth ${mpi} forecast )
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
