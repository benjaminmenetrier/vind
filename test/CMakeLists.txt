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
    if( ECSABER )
        ecbuild_add_test( TARGET vind_test_${name}_${mpi}
                          COMMAND ${CMAKE_BINARY_DIR}/bin/vind_compare.py
                          ARGS ${mpi}
                               1
                               ${CMAKE_BINARY_DIR}/bin/vind_${exec}.x
                               testinput/ec/${name}.json
                          DEPENDS vind_${exec}.x )
    else()
        ecbuild_add_test( TARGET   vind_test_${name}_${mpi}
                          MPI      ${mpi}
                          COMMAND  vind_${exec}.x
                          ARGS     testinput/jedi/${name}.yaml
                          DEPENDS  vind_${exec}.x )
    endif()
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
    
    )

    link_files( $ENV{MONARCH_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${aq_data} )
endif()

# Create the input/output directories
# -----------------------------------
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput/ec )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput/jedi )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testref )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testref/ec )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testref/jedi )

# Test input scripts
# ------------------
list( APPEND vind_testinput

# EC

# BGOS
testinput/ec/bgos_background.json
testinput/ec/bgos_ensemble.json
testinput/ec/bgos_linear_ensemble.json
testinput/ec/bgos_linear_randomized.json
testinput/ec/bgos_makeobs.json
testinput/ec/bgos_nonlinear_ensemble.json
testinput/ec/bgos_nonlinear_randomized.json
testinput/ec/bgos_stddev.json
testinput/ec/bgos_truth.json

# EVIL
testinput/ec/evil_assim_ensemble.json
testinput/ec/evil_background.json
testinput/ec/evil_control_3dvar.json
testinput/ec/evil_control_update_D.json
testinput/ec/evil_control_update_R.json
testinput/ec/evil_control_update_S.json
testinput/ec/evil_dual_3dvar.json
testinput/ec/evil_dual_update_D.json
testinput/ec/evil_dual_update_R.json
testinput/ec/evil_dual_update_S.json
testinput/ec/evil_ensemble.json
testinput/ec/evil_ensemble_variance.json
testinput/ec/evil_makeobs.json
testinput/ec/evil_primal_3dvar.json
testinput/ec/evil_primal_update_D.json
testinput/ec/evil_primal_update_R.json
testinput/ec/evil_primal_update_S.json
testinput/ec/evil_stddev.json
testinput/ec/evil_truth.json

# GLOBAL
testinput/ec/glb_3dvar.json
testinput/ec/glb_3densvar.json
testinput/ec/glb_4densvar.json
testinput/ec/glb_4denvar.json
testinput/ec/glb_background_06.json
testinput/ec/glb_background_12.json
testinput/ec/glb_background_18.json
testinput/ec/glb_ensemble_06.json
testinput/ec/glb_ensemble_12.json
testinput/ec/glb_ensemble_18.json
testinput/ec/glb_getkf_nonlinear.json
testinput/ec/glb_hofx.json
testinput/ec/glb_interfaces.json
testinput/ec/glb_makeobs_06.json
testinput/ec/glb_makeobs_12.json
testinput/ec/glb_makeobs_18.json
testinput/ec/glb_stddev.json
testinput/ec/glb_truth_06.json
testinput/ec/glb_truth_12.json
testinput/ec/glb_truth_18.json

# REGIONAL
testinput/ec/reg_3dvar.json
testinput/ec/reg_3densvar.json
testinput/ec/reg_4densvar.json
testinput/ec/reg_4denvar.json
testinput/ec/reg_background_06.json
testinput/ec/reg_background_12.json
testinput/ec/reg_background_18.json
testinput/ec/reg_ensemble_06.json
testinput/ec/reg_ensemble_12.json
testinput/ec/reg_ensemble_18.json
testinput/ec/reg_getkf_nonlinear.json
testinput/ec/reg_hofx.json
testinput/ec/reg_interfaces.json
testinput/ec/reg_makeobs_06.json
testinput/ec/reg_makeobs_12.json
testinput/ec/reg_makeobs_18.json
testinput/ec/reg_stddev.json
testinput/ec/reg_truth_06.json
testinput/ec/reg_truth_12.json
testinput/ec/reg_truth_18.json

# SYNTHETIC DATA
testinput/ec/synthetic_data_generate.json
testinput/ec/synthetic_data_makeobs.json
testinput/ec/synthetic_data_stddev.json
testinput/ec/synthetic_data_truth.json

# JEDI

# GENINT
testinput/jedi/genint_convertstate_nonuni_lonlat.yaml
testinput/jedi/genint_convertstate_regular_lonlat.yaml
testinput/jedi/genint_convertstate_varcha_regular_lonlat.yaml
testinput/jedi/genint_convertstate_varcha_nonuni_lonlat.yaml
testinput/jedi/genint_geometry_lambertCC.yaml
testinput/jedi/genint_geometry_regular_lonlat.yaml
testinput/jedi/genint_hofx3d_lambertCC.yaml
testinput/jedi/genint_hofx3d_nonuni_lonlat.yaml
testinput/jedi/genint_hofx3d_regular_lonlat.yaml

# GLOBAL
testinput/jedi/glb_3dvar.yaml
testinput/jedi/glb_3densvar.yaml
testinput/jedi/glb_4densvar.yaml
testinput/jedi/glb_4densvar_parallel.yaml
testinput/jedi/glb_background_06.yaml
testinput/jedi/glb_background_12.yaml
testinput/jedi/glb_background_18.yaml
testinput/jedi/glb_ensemble_06.yaml
testinput/jedi/glb_ensemble_12.yaml
testinput/jedi/glb_ensemble_18.yaml
testinput/jedi/glb_getkf_nonlinear.yaml
testinput/jedi/glb_letkf_linear.yaml
testinput/jedi/glb_letkf_linear_4d.yaml
testinput/jedi/glb_letkf_nonlinear.yaml
testinput/jedi/glb_letkf_nonlinear_4d.yaml
testinput/jedi/glb_letkf_read_members.yaml
testinput/jedi/glb_makeobs_06.yaml
testinput/jedi/glb_makeobs_12.yaml
testinput/jedi/glb_makeobs_18.yaml
testinput/jedi/glb_stddev.yaml
testinput/jedi/glb_truth_06.yaml
testinput/jedi/glb_truth_12.yaml
testinput/jedi/glb_truth_18.yaml

# REGIONAL
testinput/jedi/reg_3dvar.yaml
testinput/jedi/reg_3densvar.yaml
testinput/jedi/reg_4densvar.yaml
testinput/jedi/reg_4densvar_parallel.yaml
testinput/jedi/reg_background_06.yaml
testinput/jedi/reg_background_12.yaml
testinput/jedi/reg_background_18.yaml
testinput/jedi/reg_ensemble_06.yaml
testinput/jedi/reg_ensemble_12.yaml
testinput/jedi/reg_ensemble_18.yaml
testinput/jedi/reg_getkf_nonlinear.yaml
testinput/jedi/reg_letkf_linear.yaml
testinput/jedi/reg_letkf_linear_4d.yaml
testinput/jedi/reg_letkf_nonlinear.yaml
testinput/jedi/reg_letkf_nonlinear_4d.yaml
testinput/jedi/reg_letkf_read_members.yaml
testinput/jedi/reg_makeobs_06.yaml
testinput/jedi/reg_makeobs_12.yaml
testinput/jedi/reg_makeobs_18.yaml
testinput/jedi/reg_stddev.yaml
testinput/jedi/reg_truth_06.yaml
testinput/jedi/reg_truth_12.yaml
testinput/jedi/reg_truth_18.yaml
)

list( APPEND monarch_testinput
# MONARCH REGIONAL
testinput/jedi/monarch_geometry_rotated.yaml
testinput/jedi/monarch_state.yaml
testinput/jedi/monarch_convertstate.yaml
testinput/jedi/monarch_convertstate_levsel.yaml
testinput/jedi/monarch_convertstate_levsel_scaling.yaml
testinput/jedi/monarch_convertstate_levsel_logtransf.yaml
testinput/jedi/monarch_increment.yaml
testinput/jedi/monarch_ens_mean_variance.yaml
testinput/jedi/monarch_ufo_variable_name_map.yaml
testinput/jedi/monarch_hofx3d_constant.yaml
testinput/jedi/monarch_hofx3d_air_pressure.yaml
testinput/jedi/monarch_hofx3d_logarithm.yaml
testinput/jedi/monarch_dirac_no_loc_3d.yaml
testinput/jedi/monarch_dirac_no_loc_3d_lt.yaml
testinput/jedi/monarch_dirac_no_loc_4d_id.yaml
testinput/jedi/monarch_dirac_no_loc_4d.yaml
testinput/jedi/monarch_dirac_3d.yaml
testinput/jedi/monarch_dirac_4d.yaml
testinput/jedi/monarch_dirac_4d_FL.yaml
testinput/jedi/monarch_3densvar.yaml
testinput/jedi/monarch_3densvar_lt.yaml
testinput/jedi/monarch_4densvar.yaml
testinput/jedi/monarch_4densvar_lt.yaml
)

list( APPEND aq_testinput
# MONARCH REGIONAL
testinput/jedi/aq_geometry.yaml
testinput/jedi/aq_state.yaml
testinput/jedi/aq_convertstate.yaml
testinput/jedi/aq_convertstate_logtransf.yaml
testinput/jedi/aq_increment.yaml
testinput/jedi/aq_ens_mean_variance.yaml
testinput/jedi/monarch_ufo_variable_name_map.yaml
testinput/jedi/aq_hofx3d_constant.yaml
testinput/jedi/aq_hofx3d_logarithm.yaml
testinput/jedi/aq_dirac_no_loc_3d.yaml
testinput/jedi/aq_dirac_no_loc_3d_lt.yaml
testinput/jedi/aq_dirac_no_loc_4d_id.yaml
testinput/jedi/aq_dirac_no_loc_4d.yaml
testinput/jedi/aq_dirac_3d.yaml
testinput/jedi/aq_dirac_4d.yaml
testinput/jedi/aq_dirac_4d_FL.yaml
testinput/jedi/aq_3densvar.yaml
testinput/jedi/aq_3densvar_lt.yaml
testinput/jedi/aq_4densvar.yaml
testinput/jedi/aq_4densvar_lt.yaml
)

# Test output references
# ----------------------
list( APPEND vind_testref

# COMMON

# GLOBAL
testref/glb_background_06.ref
testref/glb_background_12.ref
testref/glb_background_18.ref
testref/glb_ensemble_06.ref
testref/glb_ensemble_12.ref
testref/glb_ensemble_18.ref
testref/glb_makeobs_06.ref
testref/glb_makeobs_12.ref
testref/glb_makeobs_18.ref
testref/glb_stddev.ref
testref/glb_truth_06.ref
testref/glb_truth_12.ref
testref/glb_truth_18.ref

# REGIONAL
testref/reg_background_06.ref
testref/reg_background_12.ref
testref/reg_background_18.ref
testref/reg_ensemble_06.ref
testref/reg_ensemble_12.ref
testref/reg_ensemble_18.ref
testref/reg_makeobs_06.ref
testref/reg_makeobs_12.ref
testref/reg_makeobs_18.ref
testref/reg_stddev.ref
testref/reg_truth_06.ref
testref/reg_truth_12.ref
testref/reg_truth_18.ref

# EC

# BGOS
testref/ec/bgos_background.ref
testref/ec/bgos_ensemble.ref
testref/ec/bgos_linear_ensemble.ref
testref/ec/bgos_linear_randomized.ref
testref/ec/bgos_makeobs.ref
testref/ec/bgos_nonlinear_ensemble.ref
testref/ec/bgos_nonlinear_randomized.ref
testref/ec/bgos_stddev.ref
testref/ec/bgos_truth.ref

# EVIL
testref/ec/evil_assim_ensemble.ref
testref/ec/evil_background.ref
testref/ec/evil_control_3dvar.ref
testref/ec/evil_control_update_D.ref
testref/ec/evil_control_update_R.ref
testref/ec/evil_control_update_S.ref
testref/ec/evil_dual_3dvar.ref
testref/ec/evil_dual_update_D.ref
testref/ec/evil_dual_update_R.ref
testref/ec/evil_dual_update_S.ref
testref/ec/evil_ensemble.ref
testref/ec/evil_ensemble_variance.ref
testref/ec/evil_makeobs.ref
testref/ec/evil_primal_3dvar.ref
testref/ec/evil_primal_update_D.ref
testref/ec/evil_primal_update_R.ref
testref/ec/evil_primal_update_S.ref
testref/ec/evil_stddev.ref
testref/ec/evil_truth.ref

# GLOBAL
testref/ec/glb_3dvar.ref
testref/ec/glb_3densvar.ref
testref/ec/glb_4densvar.ref
testref/ec/glb_4denvar.ref
testref/ec/glb_getkf_nonlinear.ref
testref/ec/glb_hofx.ref

# REGIONAL
testref/ec/reg_3dvar.ref
testref/ec/reg_3densvar.ref
testref/ec/reg_4densvar.ref
testref/ec/reg_4denvar.ref
testref/ec/reg_getkf_nonlinear.ref
testref/ec/reg_hofx.ref

# SYNTHETIC DATA
testref/ec/synthetic_data_generate.ref
testref/ec/synthetic_data_makeobs.ref
testref/ec/synthetic_data_stddev.ref
testref/ec/synthetic_data_truth.ref

# JEDI

# GENINT
testref/jedi/genint_geometry_regular_lonlat.ref
testref/jedi/genint_geometry_lambertCC.ref
testref/jedi/genint_convertstate_regular_lonlat.ref
testref/jedi/genint_convertstate_nonuni_lonlat.ref
testref/jedi/genint_convertstate_varcha_regular_lonlat.ref
testref/jedi/genint_convertstate_varcha_nonuni_lonlat.ref
testref/jedi/genint_hofx3d_regular_lonlat.ref
testref/jedi/genint_hofx3d_nonuni_lonlat.ref
testref/jedi/genint_hofx3d_lambertCC.ref

# GLOBAL
testref/jedi/glb_3dvar.ref
testref/jedi/glb_3densvar.ref
testref/jedi/glb_4densvar.ref
testref/jedi/glb_4densvar_parallel.ref
testref/jedi/glb_getkf_nonlinear.ref
testref/jedi/glb_letkf_linear.ref
testref/jedi/glb_letkf_linear_4d.ref
testref/jedi/glb_letkf_nonlinear.ref
testref/jedi/glb_letkf_nonlinear_4d.ref
testref/jedi/glb_letkf_read_members.ref

# REGIONAL
testref/jedi/reg_3dvar.ref
testref/jedi/reg_3densvar.ref
testref/jedi/reg_4densvar.ref
testref/jedi/reg_4densvar_parallel.ref
testref/jedi/reg_getkf_nonlinear.ref
testref/jedi/reg_letkf_linear.ref
testref/jedi/reg_letkf_linear_4d.ref
testref/jedi/reg_letkf_nonlinear.ref
testref/jedi/reg_letkf_nonlinear_4d.ref
testref/jedi/reg_letkf_read_members.ref
)

list( APPEND monarch_testref
# MONARCH REGIONAL
testref/jedi/monarch_geometry_rotated.ref
testref/jedi/monarch_state.ref
testref/jedi/monarch_convertstate.ref
testref/jedi/monarch_convertstate_levsel.ref
testref/jedi/monarch_convertstate_levsel_scaling.ref
testref/jedi/monarch_convertstate_levsel_logtransf.ref
testref/jedi/monarch_increment.ref
testref/jedi/monarch_ens_mean_variance.ref
testref/jedi/monarch_hofx3d_constant.ref
testref/jedi/monarch_hofx3d_air_pressure.ref
testref/jedi/monarch_hofx3d_logarithm.ref
testref/jedi/monarch_dirac_no_loc_3d.ref
testref/jedi/monarch_dirac_no_loc_3d_lt.ref
testref/jedi/monarch_dirac_no_loc_4d_id.ref
testref/jedi/monarch_dirac_no_loc_4d.ref
testref/jedi/monarch_dirac_3d.ref
testref/jedi/monarch_dirac_4d.ref
testref/jedi/monarch_dirac_4d_FL.ref
testref/jedi/monarch_3densvar.ref
testref/jedi/monarch_3densvar_lt.ref
testref/jedi/monarch_4densvar.ref
testref/jedi/monarch_4densvar_lt.ref
)

list( APPEND aq_testref
# MONARCH REGIONAL
testref/jedi/aq_geometry.ref
testref/jedi/aq_state.ref
testref/jedi/aq_convertstate.ref
testref/jedi/aq_convertstate_logtransf.ref
testref/jedi/aq_increment.ref
testref/jedi/aq_ens_mean_variance.ref
testref/jedi/aq_hofx3d_constant.ref
testref/jedi/aq_hofx3d_logarithm.ref
testref/jedi/aq_dirac_no_loc_3d.ref
testref/jedi/aq_dirac_no_loc_3d_lt.ref
testref/jedi/aq_dirac_no_loc_4d_id.ref
testref/jedi/aq_dirac_no_loc_4d.ref
testref/jedi/aq_dirac_3d.ref
testref/jedi/aq_dirac_4d.ref
testref/jedi/aq_dirac_4d_FL.ref
testref/jedi/aq_3densvar.ref
testref/jedi/aq_3densvar_lt.ref
testref/jedi/aq_4densvar.ref
testref/jedi/aq_4densvar_lt.ref
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

if( ( NOT ECSABER ) AND ENABLE_GENINT_TESTS )
    # GENINT tests
    create_test( genint_geometry_regular_lonlat 6 geometry )
    create_test( genint_geometry_lambertCC 6 geometry )
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

if( ( NOT ECSABER ) AND ENABLE_MONARCH_TESTS )
    # MONARCH BSC tests
    create_test( monarch_geometry_rotated 8 geometry )
    create_test( monarch_state 8 state )
    create_test( monarch_convertstate 8 convertstate )
    create_test( monarch_convertstate_levsel 8 convertstate )
    create_test( monarch_convertstate_levsel_scaling 8 convertstate )
    create_test( monarch_convertstate_levsel_logtransf 8 convertstate )
    create_test( monarch_increment 8 increment )
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
        create_test( monarch_dirac_4d 12 error_covariance_toolbox )
        create_test( monarch_dirac_4d_FL 12 error_covariance_toolbox )
    endif()

    if ( ENABLE_SABER AND ENABLE_OBS )
        create_test( monarch_3densvar 8 variational )
        create_test( monarch_3densvar_lt 8 variational )
        create_test( monarch_4densvar 12 variational )
        create_test( monarch_4densvar_lt 12 variational )
    endif()
endif()

if( ( NOT ECSABER ) AND ENABLE_AQ_TESTS )
    # AQ BSC tests
    create_test( aq_geometry 8 geometry )
    create_test( aq_state 8 state )
    create_test( aq_convertstate 8 convertstate )
    create_test( aq_convertstate_logtransf 8 convertstate )
    create_test( aq_increment 8 increment )
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
                if( ECSABER )
                    create_test( ${domain}_makeobs_${hh} ${mpi} makeobs_patched )
                else()
                    create_test( ${domain}_makeobs_${hh} ${mpi} hofx3d )
                endif()
            endif()
        endforeach()

        if( ENABLE_SABER AND ENABLE_OBS )
            create_test( ${domain}_3dvar ${mpi} variational )
            create_test( ${domain}_3densvar ${mpi} variational )
            create_test( ${domain}_4densvar ${mpi} variational )

            if( ECSABER )
                create_test( ${domain}_4denvar ${mpi} variational )
            else()
                math( EXPR mpi_par "${mpi}*3" )
                create_test( ${domain}_4densvar_parallel ${mpi_par} variational )
                create_test( ${domain}_getkf_nonlinear ${mpi} letkf )
                create_test( ${domain}_letkf_linear ${mpi} letkf )
                create_test( ${domain}_letkf_linear_4d ${mpi} letkf )
                create_test( ${domain}_letkf_nonlinear ${mpi} letkf )
                create_test( ${domain}_letkf_nonlinear_4d ${mpi} letkf )
                create_test( ${domain}_letkf_read_members ${mpi} letkf )
            endif()
        endif()
    endforeach()

    if( ECSABER )
        if( ENABLE_BGOS_TESTS )
            # BGOS tests
            create_test( bgos_truth ${mpi} convertstate )
            create_test( bgos_stddev ${mpi} convertstate )

            if( ENABLE_SABER )
                create_test( bgos_background ${mpi} error_covariance_toolbox )
                create_test( bgos_ensemble ${mpi} error_covariance_toolbox )
            endif()

            if( ENABLE_OBS )
                create_test( bgos_makeobs ${mpi} makeobs_patched )
            endif()

            if( ENABLE_SABER AND ENABLE_OBS )
                create_test( bgos_linear_ensemble ${mpi} bgos )
                create_test( bgos_linear_randomized ${mpi} bgos )
                create_test( bgos_nonlinear_ensemble ${mpi} bgos )
                create_test( bgos_nonlinear_randomized ${mpi} bgos )
            endif()
        endif()

        if( ENABLE_EVIL_TESTS )
            # EVIL tests
            create_test( evil_truth ${mpi} convertstate )
            create_test( evil_stddev ${mpi} convertstate )

            if( ENABLE_SABER )
                create_test( evil_background ${mpi} error_covariance_toolbox )
                create_test( evil_ensemble ${mpi} error_covariance_toolbox )
            endif()

            if( ENABLE_OBS )
                create_test( evil_makeobs ${mpi} makeobs_patched )
            endif()

            if( ENABLE_SABER AND ENABLE_OBS )
                create_test( evil_control_3dvar ${mpi} variational )
                create_test( evil_control_update_D ${mpi} evil_update )
                create_test( evil_control_update_R ${mpi} evil_update )
                create_test( evil_control_update_S ${mpi} evil_update )
                create_test( evil_dual_3dvar ${mpi} variational )
                create_test( evil_dual_update_D ${mpi} evil_update )
                create_test( evil_dual_update_R ${mpi} evil_update )
                create_test( evil_dual_update_S ${mpi} evil_update )
                create_test( evil_primal_3dvar ${mpi} variational )
                create_test( evil_primal_update_D ${mpi} evil_update )
                create_test( evil_primal_update_R ${mpi} evil_update )
                create_test( evil_primal_update_S ${mpi} evil_update )
                create_test( evil_assim_ensemble ${mpi} assim_ensemble_patched )
                create_test( evil_ensemble_variance ${mpi} ensemble_variance )
            endif()
        endif()

        if( ENABLE_SYNTHETIC_DATA_TESTS )
            # SYNTHETIC DATA tests
            create_test( synthetic_data_truth ${mpi} convertstate )
            create_test( synthetic_data_stddev ${mpi} convertstate )

            if( ENABLE_OBS )
                create_test( synthetic_data_makeobs ${mpi} makeobs_patched )
            endif()

            if( ENABLE_SABER AND ENABLE_OBS )
                create_test( synthetic_data_generate ${mpi} synthetic_data )
            endif()
        endif()
    endif()
endforeach()
