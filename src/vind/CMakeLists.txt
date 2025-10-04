# (C) Copyright 2022 UCAR.
# (C) Copyright 2025 Meteorologisk Institutt
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

# Common sources, not obs-related
list( APPEND vind_src_files

Constants.cc
Constants.h
Covariance.h
Fields.cc
Fields.h
Geometry.cc
Geometry.h
GeometryIterator.cc
GeometryIterator.h
GeometryParameters.h
Increment.cc
Increment.h
Interpolation.cc
Interpolation.h
LinearModel.cc
LinearModel.h
LinearVariableChange.cc
LinearVariableChange.h
LinearVariableChangeParameters.h
Model.cc
Model.h
ModelAuxControl.h
ModelAuxCovariance.h
ModelAuxIncrement.h
ModelData.h
State.cc
State.h
Traits.h
Utilities.cc
Utilities.h
VaderCookbook.h
VariableChange.cc
VariableChange.h
VariableChangeParameters.h

FieldsIO/FieldsIOArome.cc
FieldsIO/FieldsIOArome.h
FieldsIO/FieldsIOBase.cc
FieldsIO/FieldsIOBase.h
FieldsIO/FieldsIOBSC.cc
FieldsIO/FieldsIOBSC.h
FieldsIO/FieldsIOAQ.cc
FieldsIO/FieldsIOAQ.h
FieldsIO/FieldsIODefault.cc
FieldsIO/FieldsIODefault.h
FieldsIO/FieldsIOGmsh.cc
FieldsIO/FieldsIOGmsh.h

LinearModel/LinearModelBase.cc
LinearModel/LinearModelBase.h
LinearModel/LinearModelDDL95.cc
LinearModel/LinearModelDDL95.h

Model/ModelBase.cc
Model/ModelBase.h
Model/ModelDDL95.cc
Model/ModelDDL95.h

)

message( STATUS "Found vind I/O formats:" )
message( STATUS "- AROME-NetCDF" )
message( STATUS "- BSC" )
message( STATUS "- AQ" )
message( STATUS "- Default (NetCDF)" )
message( STATUS "- GMSH" )

if( ( ECTRANS_FOUND AND ectrans_HAVE_ETRANS AND ectrans_HAVE_TRANSI ) AND falfilfa_FOUND )
    # AROME FA IO-specific sources
    message( STATUS "- AROME-FA" )
    list( APPEND vind_src_files

    FieldsIO/fieldsio_arome_fa.h
    FieldsIO/fieldsio_arome_fa_interface.F90
    FieldsIO/fieldsio_arome_fa_mod.F90

    )
endif()

if( eccodes_FOUND )
    # GRIB IO-specific sources
    message( STATUS "- GRIB" )
    list( APPEND vind_src_files

    FieldsIO/FieldsIOGrib.cc
    FieldsIO/FieldsIOGrib.h

    )
endif()

message( STATUS "Found vind models:" )
message( STATUS "- DDL95" )

if( pybind11_FOUND )
    # Python-based model
    message( STATUS "- Python" )
    list( APPEND vind_src_files

#    LinearModel/LinearModelPython.cc
#    LinearModel/LinearModelPython.h
#    Model/ModelPython.cc
#    Model/ModelPython.h

    )

    # Python modules
    list( APPEND python_modules

#    ModelPythonDDL95.py

    )
endif()

ecbuild_add_library( TARGET   vind
                     SOURCES  ${vind_src_files}
                     PUBLIC_LIBS  ${VIND_PUBLIC_LIBS}
                     HEADER_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/vind
                     INSTALL_HEADERS LISTED
                     LINKER_LANGUAGE CXX )

target_link_libraries( vind PUBLIC NetCDF::NetCDF_Fortran NetCDF::NetCDF_C )
if( eccodes_FOUND )
    target_link_libraries( vind PUBLIC eccodes )
endif()
if( ( ECTRANS_FOUND AND ectrans_HAVE_ETRANS ) AND falfilfa_FOUND )
    target_link_libraries( vind PUBLIC trans_dp transi_dp fa_dp )
endif()
if( pybind11_FOUND )
    target_link_libraries( vind PUBLIC pybind11::embed )
endif()

if( pybind11_FOUND )
    # Link python modules
    message(STATUS "Available Python modules for model propagation:")
    foreach( pyMod ${python_modules} )
        execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/Python/${pyMod} ${CMAKE_BINARY_DIR}/bin/${pyMod} )
        message(STATUS "- ${CMAKE_BINARY_DIR}/${pyMod}")
    endforeach()
endif()

#Configure include directory layout for build-tree to match install-tree
set(VIND_BUILD_DIR_INCLUDE_PATH ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/include)
add_custom_target(vind_headers ALL COMMAND ${CMAKE_COMMAND} -E make_directory "${VIND_BUILD_DIR_INCLUDE_PATH}"
                                   COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR} ${VIND_BUILD_DIR_INCLUDE_PATH}/vind )
add_dependencies(vind vind_headers)
target_include_directories(vind PUBLIC $<BUILD_INTERFACE:${VIND_BUILD_DIR_INCLUDE_PATH}>
                                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
                                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../..>)
