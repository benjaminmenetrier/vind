# (C) Copyright 2024 Meteorologisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

################################################################################
# VIND
################################################################################

cmake_minimum_required( VERSION 3.23 )

project( vind VERSION 0.0.1 LANGUAGES C CXX Fortran )

################################################################################

option(OPENMP "Build vind with OpenMP support" ON)

set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH})

set( CMAKE_DIRECTORY_LABELS "vind" )

find_package( ecbuild 3.6 REQUIRED )

set( ECBUILD_DEFAULT_BUILD_TYPE Release )
set( ENABLE_OS_TESTS           OFF CACHE BOOL "Disable OS tests" FORCE )
set( ENABLE_LARGE_FILE_SUPPORT OFF CACHE BOOL "Disable testing of large file support" FORCE )
set( ENABLE_MPI ON CACHE BOOL "Compile with MPI" )
set( ENABLE_OBS ON CACHE BOOL "Enable observations" )

include( ecbuild_system NO_POLICY_SCOPE )

ecbuild_requires_macro_version( 2.5 )

################################################################################
# Project
################################################################################

ecbuild_declare_project()

ecbuild_enable_fortran( REQUIRED )

set( VIND_LINKER_LANGUAGE CXX )

################################################################################
# Compiler flags
################################################################################

list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )
set( CMAKE_DIRECTORY_LABELS ${PROJECT_NAME} )
include( ${PROJECT_NAME}_compiler_flags )

################################################################################
# Dependencies
################################################################################

# jedi-cmake
find_package( jedicmake QUIET )

# NetCDF
find_package( NetCDF REQUIRED COMPONENTS C Fortran )

# atlas
find_package( atlas 0.35.0 REQUIRED )
if( atlas_VERSION VERSION_GREATER "0.41" )
    add_compile_definitions( ATLAS_MAKE_SPARSE )
endif()

# eccodes
find_package( eccodes QUIET )

# ectrans and falfilfa
find_package( ECTRANS 1.6.0 COMPONENTS etrans double QUIET )
find_package( falfilfa QUIET )
if( ( ECTRANS_FOUND AND ectrans_HAVE_ETRANS AND ectrans_HAVE_TRANSI ) AND falfilfa_FOUND )
    add_compile_definitions( READFA )
endif()

# oops
find_package( oops 1.10.0 REQUIRED )
list( APPEND VIND_PUBLIC_LIBS "oops" )

# vader
find_package( vader 1.7.0 REQUIRED )
list( APPEND VIND_PUBLIC_LIBS "vader" )

# saber
if( ENABLE_SABER )
    find_package( saber 1.10.0 REQUIRED )
    list( APPEND VIND_PUBLIC_LIBS "saber" )
endif()

if( ENABLE_CRTM )
    # crtm
    find_package( crtm 2.2.3 REQUIRED )
endif()

if( ENABLE_OBS )
    # ufo
    find_package( ufo 1.10.0 REQUIRED )
    list( APPEND VIND_PUBLIC_LIBS "ufo" )
endif()

if( OPENMP )
    # OpenMP
    find_package( OpenMP COMPONENTS CXX Fortran )
endif()

# MPI
ecbuild_find_mpi( COMPONENTS CXX Fortran REQUIRED )

if( ENABLE_PYBIND11 )
    # Pybind11
    message( STATUS "Pybind11 loaded from directory: " ${PYBIND11_DIR} )
    add_subdirectory( ${PYBIND11_DIR} )
endif()

# Torch
if( DEFINED Torch_ROOT AND NOT Torch_ROOT STREQUAL "")
    # Torch root defined by the user at configure time, enable Torch
    set( ENABLE_TORCH ON )
endif()
if( ENABLE_TORCH )
    # Torch requires pybind11
    if( NOT ENABLE_PYBIND11 )
        message( FATAL_ERROR "ENABLE_TORCH requires ENABLE_PYBIND11" )
    endif()

    if( ENABLE_TORCH )
        execute_process(
            COMMAND ${VENV_DIR}/bin/pip install torch
            RESULT_VARIABLE PIP_RESULT
            OUTPUT_VARIABLE PIP_OUTPUT
        )
        if( NOT PIP_RESULT EQUAL 0 )
            message( STATUS "Torch installation output: " ${OUTPUT_VARIABLE PIP_OUTPUT} )
            message( FATAL_ERROR "Failed to install torch in virtual environment" )
        endif()

        if( DEFINED Torch_ROOT AND NOT Torch_ROOT STREQUAL "")
            # Torch root defined by the user at configure time
            message( STATUS "Torch root defined by user: " ${Torch_ROOT} )
        else()
            # Find Python version to define Torch root directory
            file( READ_SYMLINK ${VENV_PYTHON} PYTHON_VERSION )
            set( Torch_ROOT ${VENV_DIR}/lib/${PYTHON_VERSION}/site-packages/torch )
            message( STATUS "Torch root in the virtual environment: " ${Torch_ROOT} )
        endif()

        # Temporary fix until Torch is properly installed as a cmake package
        include_directories( ${Torch_ROOT}/include )
        include_directories( ${Torch_ROOT}/include/torch/csrc/api/include )

        # Use full paths to libraries instead of link_directories
        set( TORCH_LIBRARIES
               ${Torch_ROOT}/lib/libtorch.so
               ${Torch_ROOT}/lib/libtorch_cpu.so
               ${Torch_ROOT}/lib/libc10.so )
  endif()
endif()

################################################################################
# Sources
################################################################################

if( ENABLE_SABER )
    add_compile_definitions( ENABLE_SABER )
endif()

add_subdirectory( tools )
add_subdirectory( src )
add_subdirectory( test )

################################################################################
# Finalise configuration
################################################################################

# prepares a tar.gz of the sources and/or binaries
ecbuild_install_project( NAME vind )

# print the summary of the configuration
ecbuild_print_summary()
