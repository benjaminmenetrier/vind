# (C) Copyright 2017-2019 UCAR
# (C) Copyright 2024 Meteorologisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Select version
if( ECSABER )
    set( version "ec" )
else()
    set( version "jedi" )
endif()

# Libraries
set( LIBS ${VIND_PUBLIC_LIBS} )
list( APPEND LIBS "vind" )

# OOPS-only executables
ecbuild_add_executable( TARGET  vind_convertstate.x
                        SOURCES ${version}/vindConvertState.cc
                        LIBS    ${LIBS} )

if( ECSABER )
    ecbuild_add_executable( TARGET  vind_ensemble_variance.x
                            SOURCES ${version}/vindEnsembleVariance.cc
                            LIBS    ${LIBS} )
else()
    ecbuild_add_executable( TARGET  vind_geometry.x
                            SOURCES ${version}/vindGeometry.cc
                            LIBS    ${LIBS} )

    ecbuild_add_executable( TARGET  vind_state.x
                            SOURCES ${version}/vindTestState.cc
                            LIBS    ${LIBS} )

    ecbuild_add_executable( TARGET  vind_increment.x
                            SOURCES ${version}/vindTestIncrement.cc
                            LIBS    ${LIBS} )

    ecbuild_add_executable( TARGET  vind_ens_mean_variance.x
                            SOURCES ${version}/vindEnsMeanVariance.cc
                            LIBS    ${LIBS} )
endif()

# SABER-only executables
if( ENABLE_SABER )
    ecbuild_add_executable( TARGET  vind_error_covariance_toolbox.x
                            SOURCES ${version}/vindErrorCovarianceToolbox.cc
                            LIBS    ${LIBS} )

    ecbuild_add_executable( TARGET  vind_process_perts.x
                            SOURCES ${version}/vindProcessPerts.cc
                            LIBS    ${LIBS} )
endif()

# OBS executables
if( ENABLE_OBS )
    if( ECSABER )
        ecbuild_add_executable( TARGET  vind_hofx.x
                                SOURCES ${version}/vindHofX.cc
                                LIBS    ${LIBS} )

        ecbuild_add_executable( TARGET  vind_makeobs_patched.x
                                SOURCES ${version}/vindMakeObsPatched.cc
                                LIBS    ${LIBS} )
    else()
        ecbuild_add_executable( TARGET  vind_hofx3d.x
                                SOURCES ${version}/vindHofX3D.cc
                                LIBS    ${LIBS} )

        ecbuild_add_executable( TARGET  vind_letkf.x
                                SOURCES ${version}/vindLETKF.cc
                                LIBS    ${LIBS} )
    endif()
endif()

# SABER+OBS executable
if( ENABLE_SABER AND ENABLE_OBS )
    ecbuild_add_executable( TARGET  vind_variational.x
                            SOURCES ${version}/vindVariational.cc
                            LIBS    ${LIBS} )

    if( ECSABER )
        ecbuild_add_executable( TARGET  vind_evil_update.x
                                SOURCES ${version}/vindEvilUpdate.cc
                                LIBS    ${LIBS} )

        ecbuild_add_executable( TARGET  vind_assim_ensemble_patched.x
                                SOURCES ${version}/vindAssimEnsemblePatched.cc
                                LIBS    ${LIBS} )

        ecbuild_add_executable( TARGET  vind_bgos.x
                                SOURCES ${version}/vindBGOS.cc
                                LIBS    ${LIBS} )

        ecbuild_add_executable( TARGET  vind_synthetic_data.x
                                SOURCES ${version}/vindSyntheticData.cc
                                LIBS    ${LIBS} )
    endif()
endif()
