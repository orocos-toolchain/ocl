#
# The find_package macro for Orocos-RTT works best with
# cmake >= 2.6.3
#
cmake_minimum_required(VERSION 2.6.3)

#
# This creates a standard cmake project. You may extend this file with
# any cmake macro you see fit.
#
project(@pkgname@)

#
# Do setup in case of ros package, If ROS_ROOT is set, it is
# recommended to use RTT/OCL through the ros packages.
#
set (ROS_ROOT $ENV{ROS_ROOT} )
if (ROS_ROOT)
  include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)
  rosbuild_init()
  rosbuild_find_ros_package( rtt )
  set( RTT_HINTS HINTS ${rtt_PACKAGE_PATH}/install )
endif()

# Set the CMAKE_PREFIX_PATH in case you're not using Orocos through ROS
# for helping these find commands find RTT.
find_package(OROCOS-RTT REQUIRED ${RTT_HINTS})

# Defines the orocos_* cmake macros. See that file for additional
# documentation.
include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake)

#
# Components, types and plugins.
#
# The CMake 'target' names are identical to the first argument of the
# macros below, except for orocos_typegen_headers, where the target is fully
# controlled by generated code of 'typegen'.
#


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
orocos_typegen_headers(@pkgname@-types.hpp) # ...you may add multiple header files
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
# Be aware that a plugin may only have the loadRTTPlugin() function once defined in a .cpp file.
# This function is defined by the plugin and service CPP macros.
#
#orocos_service(@pkgname@-service @pkgname@-service.cpp) # ...only one service per library !
#orocos_plugin(@pkgname@-plugin @pkgname@-plugin.cpp) # ...only one plugin function per library !
#
# You may add multiple orocos_plugin/orocos_service statements.


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

#
# Provided 'uninstall' target
#
orocos_uninstall_target()
