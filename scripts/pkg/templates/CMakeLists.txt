#
# The find_package macro for Orocos-RTT works best with
# cmake >= 2.6.3
#
cmake_minimum_required(VERSION 2.6.3)

#
# This creates a standard cmake project. You may extend this file with
# any cmake macro you see fit.
#
# The CMake 'target' names are identical to the first argument of the
# macro, except for orocos_typekit, where the target is fully
# controlled by generated code of 'typegen'.
#
project(@pkgname@)

find_package(Orocos-RTT)

# Defines the orocos_* cmake macros. See that file for additional
# documentation.
include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake)

# Creates a component library lib@pkgname@-<target>.so
# and installs in the directory lib/orocos/@pkgname@/
#
orocos_component(@pkgname@ @pkgname@-component.cpp) # ...you may add multiple source files
#
# You may add multiple orocos_component statements.

#
# Building a typekit (recommended):
#
# Creates a typekit library lib@pkgname@-types-<target>.so
# and installs in the directory lib/orocos/@pkgname@/types/
#
orocos_typekit(@pkgname@-types.hpp) # ...you may add multiple header files
#
# You may only have *ONE* orocos_typekit statement !


#
# Building a normal library (optional):
#
# Creates a library libsupport-<target>.so and installs it in
# lib/
#
#orocos_library(support support.cpp) # ...you may add multiple source files
#
# You may add multiple orocos_library statements.


#
# Building a Plugin or Service (optional):
#
# Creates a plugin library lib@pkgname@-service-<target>.so or lib@pkgname@-plugin-<target>.so
# and installs in the directory lib/orocos/@pkgname@/plugins/
#
#orocos_plugin(@pkgname@-service @pkgname@-service.cpp) # ...you may add multiple source files
#orocos_plugin(@pkgname@-plugin @pkgname@-plugin.cpp) # ...you may add multiple source files
#
# You may add multiple orocos_plugin statements.


#
# Additional headers (not in typekit):
#
# Installs in the include/orocos/@pkgname@/ directory
#
orocos_install_headers( @pkgname@-component.hpp ) # ...you may add multiple header files
#
# You may add multiple orocos_install_headers statements.

#
# Generates and installs our orocos-@pkgname@-<target>.pc and manifest.xml file
#
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/orocos-@pkgname@.pc.in ${CMAKE_CURRENT_BINARY_DIR}/orocos-@pkgname@-${OROCOS_TARGET}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/orocos-@pkgname@-${OROCOS_TARGET}.pc DESTINATION  lib/pkgconfig )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/manifest.xml DESTINATION  lib/orocos/@pkgname@ )
