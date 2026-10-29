cmake_minimum_required( VERSION 3.23 )
project( pyatlas VERSION 0.0.1 LANGUAGES CXX )

find_package( ecbuild 3.6 REQUIRED )
include( ecbuild_system NO_POLICY_SCOPE )

ecbuild_declare_project()

find_package( eckit 1.24.0 REQUIRED )
find_package( atlas 0.36.0 REQUIRED )

add_subdirectory( src )
add_subdirectory( example )

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()
