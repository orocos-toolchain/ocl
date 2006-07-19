#
# Test CMake version
#
CMAKE_MINIMUM_REQUIRED(VERSION 2.2.2)

###################################################
#                                                 #
#       Start project customization section       #
#                                                 #
###################################################

PROJECT(orocos-components)

SET( VERSION 0.1 CACHE STRING "Version of Orocos Components" )
  MESSAGE( "Orocos Components version ${VERSION}" )

# If environment variable OROCOS_INSTALL_DIR is not set, use /usr/local/orocos
SET( INSTALL_DIR $ENV{OROCOS_INSTALL} )
IF( INSTALL_DIR )
  SET( CMAKE_INSTALL_PREFIX $ENV{OROCOS_INSTALL} CACHE PATH "Installation directory" FORCE)
ELSE( INSTALL_DIR )
  SET( CMAKE_INSTALL_PREFIX /usr/local/orocos CACHE PATH "Installation directory" FORCE)
ENDIF( INSTALL_DIR )
MESSAGE( "Setting installation directory to ${CMAKE_INSTALL_PREFIX}" )

SET( PROJ_SOURCE_DIR ${orocos-components_SOURCE_DIR} )
SET( PROJ_BINARY_DIR ${orocos-components_BINARY_DIR} )

SET( OROCOS_TARGET gnulinux )
SET( OROCOS_INSTALL ${CMAKE_INSTALL_PREFIX})

#SET( PROJ_INTERFACE_LIB OrocosInterfaces )

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
SUBDIRS( taskbrowser )


###########################################################
#                                                         #
# Enable testing by including the Dart module             #
#                                                         #
###########################################################

#INCLUDE(Dart)

