# (C) Copyright 2024-2025 Meteorlogisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Create test executables
add_subdirectory( mains )

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
macro( create_test model name mpi exec )
    # Link input file
    link_file( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} testinput/${name}.yaml )

    # Link reference file
    link_file( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} testref/${name}.ref )

    # Add test
    ecbuild_add_test( TARGET   vind_test_${model}_${name}_${mpi}
                      MPI      ${mpi}
                      COMMAND  vind_${exec}.x
                      ARGS     testinput/${name}.yaml
                      DEPENDS  vind_${exec}.x )
endmacro()

# Base core count for 3D and 4D tests
if(NOT VIND_TEST_CORES_FACTOR)
    set(VIND_TEST_CORES_FACTOR 1)
endif()

# Add tests
if( ENABLE_AQ_TESTS )
    add_subdirectory( aq )
endif()
if( ENABLE_GENINT_TESTS )
    add_subdirectory( genint )
endif()
if( ENABLE_GLB_TESTS )
    add_subdirectory( glb )
endif()
if( ENABLE_MONARCH_TESTS )
    add_subdirectory( monarch )
endif()
if( ENABLE_MONGLOB_TESTS )
    add_subdirectory( monglob )
endif()
if( ENABLE_REG_TESTS )
    add_subdirectory( reg )
endif()
