#
# Test CMake version
#
#CMAKE_MINIMUM_REQUIRED(VERSION 2.2)
#MARK_AS_ADVANCED( FORCE CMAKE_BACKWARDS_COMPATIBILITY )

###################################################
#                                                 #
#       Start project customization section       #
#                                                 #
###################################################

PROJECT(orocos-components)

SET( VERSION 0.1 CACHE STRING "Version of Orocos Components" )
  MESSAGE( "Orocos Components version ${VERSION}" )

# Do not rely on environment variables to find the path, because that's confusing
SET( CMAKE_INSTALL_PREFIX /usr/local/orocos CACHE PATH "Installation directory" )
MESSAGE( "Setting installation directory to ${CMAKE_INSTALL_PREFIX}" )

SET( PROJ_SOURCE_DIR ${orocos-components_SOURCE_DIR} )
SET( PROJ_BINARY_DIR ${orocos-components_BINARY_DIR} )

SET( OROCOS_TARGET gnulinux CACHE STRING "The Operating System target.")
SET( OROCOS_INSTALL ${CMAKE_INSTALL_PREFIX} CACHE PATH "The Orocos installation directory.")

SET( KDL_INSTALL ${CMAKE_INSTALL_PREFIX} CACHE PATH "The KDL installation directory.")
SET( BFL_INSTALL ${CMAKE_INSTALL_PREFIX} CACHE PATH "The BFL installation directory.")

# The name of the resulting library.
#SET( COMPONENTS_LIBRARY orocos-components CACHE STRING "Name of the resulting library")
#INSTALL_TARGETS( /lib ${COMPONENTS_LIBRARY} )

# The dependencies of the COMPONENTS_LIBRARY
SET( COMPONENTS_LIBRARY_DEPS "" CACHE STRING "Dependencies of the resulting library")

# Toggle if the whole build gets in a single library.
#Broken: SET( GLOBAL_LIBRARY ON CACHE BOOL "Build a large ${COMPONENTS_LIBRARY} library")

# Toggle if individual libs must be built.
SET( LOCAL_LIBRARY ON ) # CACHE BOOL "Build a library for each component.")

# Toggle if an executable must be built.
#Broken: SET( STANDALONE_COMPONENTS OFF CACHE BOOL "Build an executable for each component.")

###################################################
#                                                 #
#       End project customization section         #
#                                                 #
###################################################

#
# Set the build type (affects debugging symbols and optimization)
#
IF ( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE Release )
  MESSAGE( "Setting build type to '${CMAKE_BUILD_TYPE}'" )
ELSE ( NOT CMAKE_BUILD_TYPE )
  MESSAGE( "Build type set to '${CMAKE_BUILD_TYPE}' by user." )
ENDIF ( NOT CMAKE_BUILD_TYPE )

###################################################
#                                                 #
#       Do some global set-up & checking          #
#                                                 #
###################################################

INCLUDE(config/global_setup.cmake)

## Find Orocos RTT.

INCLUDE (${PROJ_SOURCE_DIR}/config/FindOrocosRTT.cmake)

## Find KDL

INCLUDE (${PROJ_SOURCE_DIR}/config/FindKDL.cmake)

## Find BFL

INCLUDE (${PROJ_SOURCE_DIR}/config/FindBFL.cmake)

## Uninstall target
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR} )

###########################################################
#                                                         #
# Look for low-level C headers, write defines to config.h #
#                                                         #
###########################################################

# Only do this once.
IF ( NOT WROTE_CONFIG_H )
  MESSAGE( "Writing config.h" )
  INCLUDE(config/write_config_h.cmake)
  SET( WROTE_CONFIG_H TRUE CACHE INTERNAL 
       "Remember that we've already written config.h" )
ELSE ( NOT WROTE_CONFIG_H )
  MESSAGE( "Not writing config.h -- wrote it previously" )
ENDIF ( NOT WROTE_CONFIG_H )

###########################################################
#                                                         #
# Look for dependencies required by individual components #
#                                                         #
###########################################################

INCLUDE(config/check_depend.cmake)
INCLUDE(config/component_rules.cmake)


LINK_DIRECTORIES( ${PROJ_BINARY_DIR}/taskbrowser ${PROJ_BINARY_DIR}/reporting ${PROJ_BINARY_DIR}/hardware ${PROJ_BINARY_DIR}/motion_control)
ADD_SUBDIRECTORY( taskbrowser )
ADD_SUBDIRECTORY( reporting )
ADD_SUBDIRECTORY( hardware )
ADD_SUBDIRECTORY( motion_control ) 
ADD_SUBDIRECTORY( viewer )
###########################################################
#                                                         #
# Enable testing by including the Dart module             #
#                                                         #
###########################################################

#INCLUDE(Dart)

