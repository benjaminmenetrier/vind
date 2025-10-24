# (C) Copyright 2025 Meteorolgisk Institutt
#

# VIND bundle for JEDI
cmake_minimum_required( VERSION 3.12 FATAL_ERROR )
find_package( ecbuild 3.6 REQUIRED HINTS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild)
project( vind-bundle VERSION 8.0.0 LANGUAGES C CXX Fortran )
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include( ecbuild_bundle )

# Default release mode
set( ECBUILD_DEFAULT_BUILD_TYPE Release )

# Options
option( ENABLE_MPI "Compile with MPI" ON )
option( ENABLE_OMP "Compile with OpenMP" ON )
option( ENABLE_CUDA "Compile with CUDA" OFF )
option( ENABLE_MKL "Compile with MKL" OFF )
option( ENABLE_TRANSI "Compile TransI C-interface to trans" ON )
option( ENABLE_ETRANS "Include Limited-Area-Model Transforms" ON )
option( ENABLE_JEDI_MODEL_DATA "Obtain saber/vader test data from jedi-model-data repository (vs tarball)" ON )
option( ENABLE_PYBIND11 "Compile with pybind11" OFF )
option( ENABLE_TORCH "Compile with torch" OFF )
option( ENABLE_IODA_DATA "Obtain ioda test data from ioda-data repository (vs tarball)" ON )
option( ENABLE_UFO_DATA "Obtain ufo test data from ufo-data repository (vs tarball)" ON )
option( BUILD_IODA_CONVERTERS "Build IODA Converters" OFF )

# What to compile?
option( ENABLE_SABER "Compile SABER" ON )
option( ENABLE_OBS "Compile IODA/UFO" ON )
option( ENABLE_CRTM "Compile CRTM" OFF )

# What tests to run?
option( ENABLE_AQ_TESTS "Run aq tests" OFF )
option( ENABLE_GENINT_TESTS "Run genint tests" ON )
option( ENABLE_GLB_TESTS "Run global tests" ON )
option( ENABLE_MONARCH_TESTS "Run regional monarch tests" OFF )
option( ENABLE_MONGLOB_TESTS "Run global monarch tests" OFF )
option( ENABLE_REG_TESTS "Run regional tests" ON )

# Setup
link_directories( ${CMAKE_CURRENT_BINARY_DIR}/lib )
include( GNUInstallDirs )
set( INSTALL_LIB_DIR ${CMAKE_INSTALL_LIBDIR} CACHE STRING "Library installation directory" FORCE )
set( ECBUILD_DEFAULT_BUILD_TYPE Release CACHE STRING "ECBuild default build type" FORCE )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH ON )
set( CMAKE_BUILD_WITH_INSTALL_RPATH ON )

# Initialize bundle
ecbuild_bundle_initialize()

# Use external jedi-cmake or build in bundle
if( DEFINED ENV{jedi_cmake_ROOT} )
    include( $ENV{jedi_cmake_ROOT}/share/jedicmake/Functions/git_functions.cmake )
    list( APPEND CMAKE_MODULE_PATH $ENV{jedi_cmake_ROOT}/share/jedicmake/Modules )
    message( INFO "CMAKE_MODULE_PATH" )
else()
    ecbuild_bundle( PROJECT jedicmake GIT "https://github.com/jcsda/jedi-cmake.git" TAG 40d521f9a2d796fcbc6234d77abceeffefb8eb7f RECURSIVE )
    include( jedicmake/cmake/Functions/git_functions.cmake )
    list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/jedicmake/CMakeModules/Modules )
endif()

# ECMWF tools
ecbuild_bundle( PROJECT eckit GIT "https://github.com/ecmwf/eckit.git" TAG 1.32.2 )
ecbuild_bundle( PROJECT fckit GIT "https://github.com/ecmwf/fckit.git" TAG 0.14.0 )
ecbuild_bundle( PROJECT fiat GIT "https://github.com/ecmwf-ifs/fiat.git" TAG 1.6.1 )
ecbuild_bundle( PROJECT ectrans GIT "https://github.com/ecmwf-ifs/ectrans.git" TAG 3094b24ec9c1862772e2b02f4e06c797247d57fc )
ecbuild_bundle( PROJECT atlas GIT "https://github.com/ecmwf/atlas.git" TAG 0.44.0 )

# oops
ecbuild_bundle( PROJECT oops GIT "https://github.com/jcsda/oops.git" TAG d73dbc828f809c7a0dd6c8044436fa99e65df547 )

# jedi-data
ecbuild_bundle( PROJECT jedi-model-data GIT "https://github.com/benjaminmenetrier/jedi-model-data.git" TAG d255823817fb5d2415b2b8a6517e0d719a14ca0d )

# vader
ecbuild_bundle( PROJECT vader GIT "https://github.com/benjaminmenetrier/vader.git" TAG 79aaae0d3928ae2f462c985d483fbd550dd75503 )

if( ENABLE_SABER )
    # gsibec
    ecbuild_bundle( PROJECT gsibec GIT "https://github.com/benjaminmenetrier/GSIbec.git" TAG a81044719ab3a5bfec72dbf6aab138b451e01ccb )


    # saber
    ecbuild_bundle( PROJECT saber GIT "https://github.com/benjaminmenetrier/saber.git" TAG e3c9b8ade414116bc81a76e70d6a4f3a31fd8f27 )
endif()

if( ENABLE_CRTM )
    # crtm
    ecbuild_bundle( PROJECT crtm GIT "https://github.com/jcsda/CRTMv3.git" TAG 524fcc623f65784dfb27277a7a35b53b1311f276 )
endif()

if( ENABLE_OBS )
    # ioda
#    ecbuild_bundle( PROJECT ioda-data GIT "https://github.com/jcsda-internal/ioda-data.git" TAG 8fb8ea112b2998ac49e4b61a733f87796924efb7 )
    ecbuild_bundle( PROJECT ioda GIT "https://github.com/jcsda/ioda.git" TAG 3a1b65488c7926e71a32a4fe03ac1b5ed48258fb )
    if( BUILD_IODA_CONVERTERS )
        ecbuild_bundle( PROJECT iodaconv GIT "https://github.com/jcsda/ioda-converters.git" TAG f794abc237046964bc9316f45698af9a0eced121 )
    endif()

    # ufo
#    ecbuild_bundle( PROJECT ufo-data GIT "https://github.com/jcsda-internal/ufo-data.git" TAG 2fa2e99503f7944274e5edd315a5cb92bf2acf8c )
    ecbuild_bundle( PROJECT ufo GIT "https://github.com/jcsda/ufo.git" TAG 61eafacd89139e60e48d84bc701b6ff075e13a39 )
endif()

# vind
ecbuild_bundle( PROJECT vind SOURCE ..)

ecbuild_bundle_finalize()
