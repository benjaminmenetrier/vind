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
set( ENABLE_MPI ON CACHE BOOL "Compile with MPI" )
set( ENABLE_OMP ON CACHE BOOL "Compile with OpenMP" )
set( ENABLE_CUDA OFF CACHE BOOL "Compile with CUDA" )
set( ENABLE_TRANSI ON CACHE BOOL "Compile TransI C-interface to trans" )
set( ENABLE_ETRANS ON CACHE BOOL "Include Limited-Area-Model Transforms" )
set( ENABLE_IODA_DATA ON CACHE BOOL "Obtain ioda test data from ioda-data repository (vs tarball)" )
set( ENABLE_UFO_DATA ON CACHE BOOL "Obtain ufo test data from ufo-data repository (vs tarball)" )
set( BUILD_IODA_CONVERTERS OFF CACHE BOOL "Build IODA Converters" )

# What to compile?
set( ENABLE_SABER ON )
set( ENABLE_OBS ON )
set( ENABLE_CRTM OFF )

# What tests to run?
set( ENABLE_GENINT_TESTS ON )
set( ENABLE_MONARCH_TESTS OFF )
set( ENABLE_AQ_TESTS OFF )
set( ENABLE_GLB_TESTS ON )
set( ENABLE_REG_TESTS ON )
set( ENABLE_BGOS_TESTS ON )
set( ENABLE_EVIL_TESTS ON )
set( ENABLE_SYNTHETIC_DATA_TESTS ON )

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
    ecbuild_bundle( PROJECT jedicmake GIT "https://github.com/jcsda/jedi-cmake.git" BRANCH develop UPDATE RECURSIVE )
    include( jedicmake/cmake/Functions/git_functions.cmake )
    list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/jedicmake/CMakeModules/Modules )
endif()

# Tools
ecbuild_bundle( PROJECT eckit GIT "https://github.com/ecmwf/eckit.git" TAG 1.25.0 )
ecbuild_bundle( PROJECT fckit GIT "https://github.com/ecmwf/fckit.git" TAG 0.11.0 )
ecbuild_bundle( PROJECT fiat GIT "https://github.com/ecmwf-ifs/fiat.git" BRANCH main )
ecbuild_bundle( PROJECT ectrans GIT "https://github.com/ecmwf-ifs/ectrans.git" BRANCH main )
ecbuild_bundle( PROJECT atlas GIT "https://github.com/ecmwf/atlas.git" TAG 0.40.0 )

# oops
ecbuild_bundle( PROJECT oops GIT "https://github.com/jcsda/oops.git" BRANCH develop UPDATE )

# vader
#ecbuild_bundle( PROJECT vader GIT "https://github.com/jcsda/vader.git" BRANCH develop UPDATE )
ecbuild_bundle( PROJECT vader GIT "https://github.com/benjaminmenetrier/vader.git" BRANCH feature/recipes_hmr_at_ap UPDATE )

if( ENABLE_SABER )
    # gsi
#    ecbuild_bundle( PROJECT gsibec GIT "https://github.com/geos-esm/GSIbec.git" BRANCH develop UPDATE )
    ecbuild_bundle( PROJECT gsibec GIT "https://github.com/benjaminmenetrier/GSIbec.git" BRANCH bugfix/fortran_line_length UPDATE )


    # saber
#    ecbuild_bundle( PROJECT saber GIT "https://github.com/jcsda/saber.git" BRANCH develop UPDATE )
    ecbuild_bundle( PROJECT saber GIT "https://github.com/benjaminmenetrier/saber.git" BRANCH feature/nicas_article UPDATE )
endif()

if( ENABLE_CRTM )
    # crtm
    ecbuild_bundle( PROJECT crtm GIT "https://github.com/jcsda/CRTMv3.git" BRANCH develop UPDATE )
endif()

if( ENABLE_OBS )
    # ioda
#    ecbuild_bundle( PROJECT ioda-data GIT "https://github.com/jcsda-internal/ioda-data.git" BRANCH develop UPDATE )
    ecbuild_bundle( PROJECT ioda GIT "https://github.com/jcsda/ioda.git" BRANCH develop UPDATE )
    if( BUILD_IODA_CONVERTERS )
        ecbuild_bundle( PROJECT iodaconv GIT "https://github.com/jcsda/ioda-converters.git" BRANCH develop UPDATE )
    endif()

    # ufo
#    ecbuild_bundle( PROJECT ufo-data GIT "https://github.com/jcsda-internal/ufo-data.git" BRANCH develop UPDATE )
    ecbuild_bundle( PROJECT ufo GIT "https://github.com/jcsda/ufo.git" BRANCH develop UPDATE )
endif()

# vind
ecbuild_bundle( PROJECT vind SOURCE ..)

ecbuild_bundle_finalize()
