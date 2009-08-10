#
# Test CMake version
#
CMAKE_MINIMUM_REQUIRED(VERSION 2.2)
#MARK_AS_ADVANCED( FORCE CMAKE_BACKWARDS_COMPATIBILITY )

# for CMake 2.6 corrected behaviour (see "cmake --help-policy CMP0003")
IF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} GREATER 4)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} GREATER 4)


###################################################
#                                                 #
#       Start project customization section       #
#                                                 #
###################################################

PROJECT(orocos-ocl)

SET( OCL_VERSION 1.8.99 )
STRING( REGEX MATCHALL "[0-9]+" OCL_VERSIONS ${OCL_VERSION} )
LIST( GET OCL_VERSIONS 0 OCL_VERSION_MAJOR)
LIST( GET OCL_VERSIONS 1 OCL_VERSION_MINOR)
LIST( GET OCL_VERSIONS 2 OCL_VERSION_PATCH)
MESSAGE( "Orocos Component Library version ${VERSION} (${OCL_VERSION_MAJOR}.${OCL_VERSION_MINOR}.${OCL_VERSION_PATCH})" )

# Do not rely on environment variables to find the path, because that's confusing
IF(NOT CMAKE_INSTALL_PREFIX)
  SET( CMAKE_INSTALL_PREFIX /usr/local CACHE PATH "Installation directory" FORCE)
  MESSAGE( "Setting installation directory to ${CMAKE_INSTALL_PREFIX}" )
ENDIF(NOT CMAKE_INSTALL_PREFIX)

SET( PROJ_SOURCE_DIR ${orocos-ocl_SOURCE_DIR} )
SET( PROJ_BINARY_DIR ${orocos-ocl_BINARY_DIR} )

IF(NOT OROCOS_INSTALL)
  SET( OROCOS_INSTALL ${CMAKE_INSTALL_PREFIX} CACHE PATH "The RTT installation directory.")
ENDIF(NOT OROCOS_INSTALL)
IF(NOT RTT_VVERSION)
  SET( RTT_VVERSION "v${OCL_VERSION_MAJOR}.${OCL_VERSION_MINOR}.x" )
ENDIF(NOT RTT_VVERSION)

IF(NOT KDL_INSTALL)
  SET( KDL_INSTALL ${CMAKE_INSTALL_PREFIX} CACHE PATH "The KDL installation directory.")
ENDIF(NOT KDL_INSTALL)
IF(NOT KDL_VVERSION)
  SET( KDL_VVERSION "v0.4.x" )
ENDIF(NOT KDL_VVERSION)

IF(NOT BFL_INSTALL)
  SET( BFL_INSTALL ${CMAKE_INSTALL_PREFIX} CACHE PATH "The BFL installation directory.")
ENDIF(NOT BFL_INSTALL)

IF(NOT COMEDI_INSTALL)
  SET( COMEDI_INSTALL ${CMAKE_INSTALL_PREFIX} CACHE PATH "The COMEDI installation directory.")
ELSE(NOT COMEDI_INSTALL)
  SET( COMEDI_INSTALL "${COMEDI_INSTALL}" CACHE PATH "The COMEDI installation directory.")
ENDIF(NOT COMEDI_INSTALL)

IF(NOT NETCDF_INSTALL)
  SET( NETCDF_INSTALL ${CMAKE_INSTALL_PREFIX} CACHE PATH "The NETCDF installation directory.")
ENDIF(NOT NETCDF_INSTALL)

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

# Make compilation verbose by default
SET(CMAKE_VERBOSE_MAKEFILE FALSE)

###################################################
#                                                 #
#       End project customization section         #
#                                                 #
###################################################

#
# Set the build type (affects debugging symbols and optimization)
#
IF ( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE "OCL" CACHE STRING "Build type: None (Use CMAKE_C_FLAGS and CMAKE_CXX_FLAGS), OCL (Recommended), Release, Debug, RelWithDebInfo, MinSizeRel." FORCE )
  MESSAGE( "Setting build type to '${CMAKE_BUILD_TYPE}'" )
  SET( CMAKE_C_FLAGS_OCL "-O2 -DNDEBUG" CACHE STRING "The CFLAGS for OCL" )
  SET( CMAKE_CXX_FLAGS_OCL "-O2 -DNDEBUG" CACHE STRING "The CXXFLAGS flags for OCL"  )
ELSE ( NOT CMAKE_BUILD_TYPE )
  MESSAGE( "Build type set to '${CMAKE_BUILD_TYPE}' by user." )
  IF ( NOT CMAKE_BUILD_TYPE STREQUAL "OCL" )
    MARK_AS_ADVANCED(FORCE CMAKE_C_FLAGS_OCL CMAKE_CXX_FLAGS_OCL)
  ELSE ( NOT CMAKE_BUILD_TYPE STREQUAL "OCL" )
    MARK_AS_ADVANCED(CLEAR CMAKE_C_FLAGS_OCL CMAKE_CXX_FLAGS_OCL)
  ENDIF ( NOT CMAKE_BUILD_TYPE STREQUAL "OCL" )

ENDIF ( NOT CMAKE_BUILD_TYPE )

# MESSAGE("Looking for compiler:")
INCLUDE (${CMAKE_ROOT}/Modules/CMakeDetermineCXXCompiler.cmake)

###################################################
#                                                 #
#       Do some global set-up & checking          #
#                                                 #
###################################################

#
# This is the first in the include path.
INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR} ${PROJ_BINARY_DIR}/ocl )

# Put your 'flags' inhere if you want them to apear in the
# orocos-components.pc file. Various .cmake scripts below do
# this.
SET( ENV{OROCOS_COMPONENTS_CFLAGS} "")
SET( ENV{OROCOS_COMPONENTS_LINKFLAGS} "")
SET( ENV{OROCOS_COMPONENTS_REQUIRES} "")

INCLUDE(${PROJ_SOURCE_DIR}/config/global_setup.cmake)
INCLUDE(${PROJ_SOURCE_DIR}/config/component_rules.cmake)

## Find Orocos RTT.

INCLUDE (${PROJ_SOURCE_DIR}/config/FindOrocosRTT.cmake)
IF (OROCOS_RTT_1.4)
  OROCOS_PKGCONFIG_REQUIRES("orocos-rtt-${OROCOS_TARGET}")
ELSE (OROCOS_RTT_1.4)
  OROCOS_PKGCONFIG_REQUIRES("orocos-rtt")
ENDIF (OROCOS_RTT_1.4)

## Find KDL

INCLUDE (${PROJ_SOURCE_DIR}/config/FindKDL.cmake)

## Find BFL

INCLUDE (${PROJ_SOURCE_DIR}/config/FindBFL.cmake)

## Find Comedi

INCLUDE (${PROJ_SOURCE_DIR}/config/FindComedi.cmake)

## Find Netcdf
INCLUDE (${PROJ_SOURCE_DIR}/config/FindNetcdf.cmake)

## Uninstall target
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/config/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

OPTION( NO_GPL "Set to 'ON' to exclude all GPL'd code" OFF)
IF(NO_GPL)
  ADD_DEFINITIONS( -DNO_GPL )
ENDIF(NO_GPL)


###########################################################
#                                                         #
# Look for dependencies required by individual components #
#                                                         #
###########################################################

INCLUDE(${PROJ_SOURCE_DIR}/config/check_depend.cmake)

#First unset the LOCAL_LIBRARIES
SET(ENV{SELECTED_LIBS} "")
SET(ENV{SELECTED_DIRS} "")
ADD_SUBDIRECTORY( doc )
ADD_SUBDIRECTORY( ocl )
ADD_SUBDIRECTORY( taskbrowser )
ADD_SUBDIRECTORY( timer )
ADD_SUBDIRECTORY( helloworld )
ADD_SUBDIRECTORY( reporting )
ADD_SUBDIRECTORY( hardware )
ADD_SUBDIRECTORY( motion_control ) 
ADD_SUBDIRECTORY( viewer )
ADD_SUBDIRECTORY( deployment )
ADD_SUBDIRECTORY( bin )

MESSAGE("Selected Libraries: $ENV{SELECTED_LIBS}")
SET(SELECTED_LIBS $ENV{SELECTED_LIBS})
SET(OROCOS_COMPONENTS_LINKFLAGS "$ENV{OROCOS_COMPONENTS_LINKFLAGS}")
SET(OROCOS_COMPONENTS_CFLAGS "$ENV{OROCOS_COMPONENTS_CFLAGS}")
SET(OROCOS_COMPONENTS_REQUIRES "$ENV{OROCOS_COMPONENTS_REQUIRES}")

CONFIGURE_FILE( ${PROJ_SOURCE_DIR}/ocl/ocl-config.h.in ${PROJ_BINARY_DIR}/ocl/ocl-config.h @ONLY)
CONFIGURE_FILE( ${PROJ_SOURCE_DIR}/orocos-ocl.pc.in ${PROJ_BINARY_DIR}/orocos-ocl-${OROCOS_TARGET}.pc @ONLY)
INSTALL_FILES( /lib/pkgconfig FILES orocos-ocl-${OROCOS_TARGET}.pc)
INSTALL_FILES( /include/ocl FILES ocl/ocl-config.h)

#MESSAGE("Selected dirs: $ENV{SELECTED_DIRS}")
#SET(SELECTED_DIRS $ENV{SELECTED_DIRS})

CONFIGURE_FILE(Doxyfile.in Doxyfile @ONLY)

############################################################
##                                                         #
## Setup PACKAGING   THIS IS EXPERIMENTAL/BETA/NOT USED    #
##                                                         #
############################################################

IF ( "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" STREQUAL "2.4" )
OPTION( CPACK_PACKAGES "Set to ON to build the packages. Requires cmake >2.4" OFF )
MARK_AS_ADVANCED( CPACK_PACKAGES )
IF (CPACK_PACKAGES)

INCLUDE(InstallRequiredSystemLibraries)

# ATTENTION: There is sometimes a _SOURCE_ version of an
# option as well, set both if necessary !

# Create .tar.gz and .tar.tbz2 files:
SET(CPACK_GENERATOR "TBZ2")
SET(CPACK_SOURCE_GENERATOR "TBZ2")

# The plain 'package' target works correctly.
SET(CPACK_IGNORE_FILES        "/CVS/;/.svn/;.swp$;.#;/#;/build/")
# Since the 'package_source' target does a bold copy, define a list of
# files which should be excluded. Note that 'subpattern' matching is used,
# thus to exclude a directory use /mydir/
SET(CPACK_SOURCE_IGNORE_FILES "/CVS/;/.svn/;.swp$;.#;/#;/build/;~")

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Orocos Components")
SET(CPACK_PACKAGE_VENDOR "The Orocos Community")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_MAJOR_VERSION "0")
SET(CPACK_PACKAGE_MINOR_VERSION "1")
SET(CPACK_PACKAGE_VERSION_PATCH "1")

SET(CPACK_PACKAGE_EXECUTABLES "OrocosExec" "Orocos Executable")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
SET(CPACK_SOURCE_TOPLEVEL_TAG "src")
SET(CPACK_PACKAGE_FILE_NAME "orocos-ocl")

INCLUDE(CPack)
ENDIF (CPACK_PACKAGES)
ELSE ( "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" STREQUAL "2.4" )

   MESSAGE ( "Disabling packaging for version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}")

ENDIF ( "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" STREQUAL "2.4" )

#############################
#			    #
# Building Doxygen documents#
#                           #
#############################

ADD_CUSTOM_TARGET(docapi "doxygen" "Doxyfile")
  

## Debugging
OPTION( DISPLAY_FLAGS "Display cmake variables at end of configure step." OFF)
MARK_AS_ADVANCED(DISPLAY_FLAGS)
IF( DISPLAY_FLAGS )
INCLUDE(config/cmake_logging.cmake)
ENDIF( DISPLAY_FLAGS )
