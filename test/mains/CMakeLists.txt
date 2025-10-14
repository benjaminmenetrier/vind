# (C) Copyright 2017-2019 UCAR
# (C) Copyright 2024 Meteorologisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Libraries
set( LIBS ${VIND_PUBLIC_LIBS} )
list( APPEND LIBS "vind" )

# Test executables
ecbuild_add_executable( TARGET  vind_test_geometry.x
                        SOURCES vindTestGeometry.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_test_geometry_iterator.x
                        SOURCES vindTestGeometryIterator.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_test_increment.x
                        SOURCES vindTestIncrement.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_test_linear_variable_change.x
                        SOURCES vindTestLinearVariableChange.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_test_model.x
                        SOURCES vindTestModel.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_test_model_aux_control.x
                        SOURCES vindTestModelAuxControl.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_test_model_aux_covariance.x
                        SOURCES vindTestModelAuxCovariance.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_test_model_aux_increment.x
                        SOURCES vindTestModelAuxIncrement.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_test_state.x
                        SOURCES vindTestState.cc
                        LIBS    ${LIBS} )

ecbuild_add_executable( TARGET  vind_test_variable_change.x
                        SOURCES vindTestVariableChange.cc
                        LIBS    ${LIBS} )

if( ENABLE_SABER )
    ecbuild_add_executable( TARGET  vind_test_error_covariance.x
                            SOURCES vindTestErrorCovariance.cc
                            LIBS    ${LIBS} )

    ecbuild_add_executable( TARGET  vind_test_linear_model.x
                            SOURCES vindTestLinearModel.cc
                            LIBS    ${LIBS} )
endif()
