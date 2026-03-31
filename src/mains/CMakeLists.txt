# (C) Copyright 2017-2019 UCAR
# (C) Copyright 2024 Meteorologisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Libraries
set( LIBS ${VIND_PUBLIC_LIBS} )
list( APPEND LIBS "vind" )

# OOPS-only executables
ecbuild_add_executable( TARGET  vind_forecast.x
                        SOURCES vindForecast.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_convertstate.x
                        SOURCES vindConvertState.cc
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

    ecbuild_add_executable( TARGET  vind_hofx4d.x
                            SOURCES vindHofX4D.cc
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
endif()

# Add RPATH for Torch libraries if found
if( ENABLE_TORCH )
    foreach( target vind_convertstate.x vind_error_covariance_toolbox.x vind_process_perts.x vind_variational.x )
        get_target_property(EXISTING_RPATH ${target} BUILD_RPATH)
        if(EXISTING_RPATH)
            set_target_properties( ${target} PROPERTIES
                BUILD_RPATH "${EXISTING_RPATH};${Torch_ROOT}/lib" )
        else()
            set_target_properties( ${target} PROPERTIES
                BUILD_RPATH "${Torch_ROOT}/lib" )
        endif()
        get_target_property(EXISTING_INSTALL_RPATH ${target} INSTALL_RPATH)
        if(EXISTING_INSTALL_RPATH)
            set_target_properties( ${target} PROPERTIES
                INSTALL_RPATH "${EXISTING_INSTALL_RPATH};${Torch_ROOT}/lib" )
        else()
            set_target_properties( ${target} PROPERTIES
                INSTALL_RPATH "${Torch_ROOT}/lib" )
        endif()
    endforeach()
endif()
