# (C) Copyright 2017-2019 UCAR
# (C) Copyright 2024 Meteorologisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Libraries
set( LIBS ${VIND_PUBLIC_LIBS} )
list( APPEND LIBS "vind" )

# OOPS-only executables
ecbuild_add_executable( TARGET  vind_convertstate.x
                        SOURCES vindConvertState.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_geometry.x
                        SOURCES vindGeometry.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_state.x
                        SOURCES vindTestState.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_increment.x
                        SOURCES vindTestIncrement.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_ens_mean_variance.x
                        SOURCES vindEnsMeanVariance.cc
                        LIBS    ${LIBS} )

# SABER-only executables
if( ENABLE_SABER )
    ecbuild_add_executable( TARGET  vind_error_covariance_toolbox.x
                            SOURCES vindErrorCovarianceToolbox.cc
                            LIBS    ${LIBS} )

    ecbuild_add_executable( TARGET  vind_process_perts.x
                            SOURCES vindProcessPerts.cc
                            LIBS    ${LIBS} )
endif()

# OBS executables
if( ENABLE_OBS )
    ecbuild_add_executable( TARGET  vind_hofx3d.x
                            SOURCES vindHofX3D.cc
                            LIBS    ${LIBS} )

    ecbuild_add_executable( TARGET  vind_letkf.x
                            SOURCES vindLETKF.cc
                            LIBS    ${LIBS} )
endif()

# SABER+OBS executable
if( ENABLE_SABER AND ENABLE_OBS )
    ecbuild_add_executable( TARGET  vind_variational.x
                            SOURCES vindVariational.cc
                            LIBS    ${LIBS} )

#    ecbuild_add_executable( TARGET  vind_bgos.x
#                            SOURCES vindBGOS.cc
#                            LIBS    ${LIBS} )
endif()
