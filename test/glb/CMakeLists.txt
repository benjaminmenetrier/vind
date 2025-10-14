# (C) Copyright 2025 Meteorlogisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Create the test input / reference / data directories
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testref )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testdata )

# Test parameters
list( APPEND mpi_list "1" "4" )

# Add tests
foreach( mpi ${mpi_list} )
    # Attractor
    create_test( glb attractor ${mpi} forecast )

    # Uniform standard-deviation
    create_test( glb stddev ${mpi} convertstate )

    # Interface tests
    create_test( glb test_error_covariance ${mpi} test_error_covariance )
    create_test( glb test_geometry ${mpi} test_geometry )
    create_test( glb test_geometry_iterator ${mpi} test_geometry_iterator )
    create_test( glb test_increment ${mpi} test_increment )
    create_test( glb test_linear_model ${mpi} test_linear_model )
    if( pybind11_FOUND )
        create_test( glb test_linear_model_numpy ${mpi} test_linear_model )
        create_test( glb test_linear_model_pytorch ${mpi} test_linear_model )
    endif()
    if( Torch_FOUND )
        create_test( glb test_linear_model_torch ${mpi} test_linear_model )
    endif()
    create_test( glb test_linear_variable_change ${mpi} test_linear_variable_change )
    create_test( glb test_model ${mpi} test_model )
    if( pybind11_FOUND )
        create_test( glb test_model_numpy ${mpi} test_model )
        create_test( glb test_model_pytorch ${mpi} test_model )
    endif()
    if( Torch_FOUND )
        create_test( glb test_model_torch ${mpi} test_model )
    endif()
    create_test( glb test_model_aux_control ${mpi} test_model_aux_control )
    create_test( glb test_model_aux_covariance ${mpi} test_model_aux_covariance )
#    create_test( glb test_model_aux_increment ${mpi} test_model_aux_increment )
    create_test( glb test_state ${mpi} test_state )
    create_test( glb test_variable_change ${mpi} test_variable_change )

    # Truth
    create_test( glb truth ${mpi} forecast )

    if( ENABLE_SABER )
        # Background
        create_test( glb background ${mpi} error_covariance_toolbox )

        # Ensemble
        create_test( glb ensemble ${mpi} error_covariance_toolbox )
    endif()

    if( ENABLE_OBS )
        # Make observations, 3d and 4d
        create_test( glb makeobs_3d ${mpi} hofx3d )
        create_test( glb makeobs_4d ${mpi} hofx4d )
    endif()

    if( ENABLE_SABER AND ENABLE_OBS )
        # 3DVar
        create_test( glb 3dvar ${mpi} variational )

        # 4DVar
        create_test( glb 4dvar ${mpi} variational )

        # 3DEnVar
        create_test( glb 3densvar ${mpi} variational )

        # 4DEnVar
        math( EXPR mpi_par "${mpi}*3" )
        create_test( glb 4densvar ${mpi} variational )
        create_test( glb 4densvar_parallel ${mpi_par} variational )

        # LETKF
        create_test( glb getkf_nonlinear ${mpi} letkf )
        create_test( glb letkf_linear ${mpi} letkf )
        create_test( glb letkf_linear_4d ${mpi} letkf )
        create_test( glb letkf_nonlinear ${mpi} letkf )
        create_test( glb letkf_nonlinear_4d ${mpi} letkf )
        create_test( glb letkf_read_members ${mpi} letkf )
     endif()
endforeach()
