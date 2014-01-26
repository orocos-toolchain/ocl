#
# Test CMake version
#
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
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

PROJECT(ocl)

SET( OCL_VERSION 2.7.0 )
STRING( REGEX MATCHALL "[0-9]+" OCL_VERSIONS ${OCL_VERSION} )
LIST( GET OCL_VERSIONS 0 OCL_VERSION_MAJOR)
LIST( GET OCL_VERSIONS 1 OCL_VERSION_MINOR)
LIST( GET OCL_VERSIONS 2 OCL_VERSION_PATCH)
SET( COMPONENT_VERSION ${OCL_VERSION} ) # Necessary for UseOrocos macros.
MESSAGE( "Orocos Component Library version ${VERSION} (${OCL_VERSION_MAJOR}.${OCL_VERSION_MINOR}.${OCL_VERSION_PATCH})" )

SET( PROJ_SOURCE_DIR ${ocl_SOURCE_DIR} )
SET( PROJ_BINARY_DIR ${ocl_BINARY_DIR} )

# Load the optional orocos-ocl.cmake file.
INCLUDE(${PROJ_SOURCE_DIR}/orocos-ocl.cmake OPTIONAL)
if (NOT EXISTS ${PROJ_SOURCE_DIR}/orocos-ocl.cmake)
  # Setup the default settings
  INCLUDE(${PROJ_SOURCE_DIR}/orocos-ocl.default.cmake)
  MESSAGE(
    "No orocos-ocl.cmake file loaded, using default settings."
    "See orocos-ocl.default.cmake")
endif ()

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  IF (MSVC)
    SET(CMAKE_INSTALL_PREFIX
      "C:/Program Files/orocos" CACHE PATH "Orocos install prefix" FORCE
      )
  ENDIF(MSVC)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

INCLUDE(CTest)
ENABLE_TESTING()
IF(BUILD_TESTING)
  SET(BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  MARK_AS_ADVANCED(BUILDNAME)
ENDIF(BUILD_TESTING)

# Do not rely on environment variables to find the path, because that's confusing
IF(NOT CMAKE_INSTALL_PREFIX)
  SET( CMAKE_INSTALL_PREFIX /usr/local CACHE PATH "Installation directory" FORCE)
  MESSAGE( "Setting installation directory to ${CMAKE_INSTALL_PREFIX}" )
ENDIF(NOT CMAKE_INSTALL_PREFIX)

IF(NOT NETCDF_INSTALL)
  SET( NETCDF_INSTALL ${CMAKE_INSTALL_PREFIX} CACHE PATH "The NETCDF installation directory.")
ENDIF(NOT NETCDF_INSTALL)

# The dependencies of the COMPONENTS_LIBRARY
SET( COMPONENTS_LIBRARY_DEPS "" CACHE STRING "Dependencies of the resulting library")

# Make compilation verbose by default
SET(CMAKE_VERBOSE_MAKEFILE FALSE)

# suffix for orocos/ install dir.
if (NOT OROCOS_SUFFIX)
  set(OROCOS_SUFFIX)
endif(NOT OROCOS_SUFFIX)

###################################################
#                                                 #
#       End project customization section         #
#                                                 #
###################################################

if(OROCOS_TARGET STREQUAL "win32")
  if (MSVC)
    if (NOT MSVC80)
      set(NUM_PARALLEL_BUILD 4 CACHE STRING "Number of parallel builds")
      set(PARALLEL_FLAG "/MP${NUM_PARALLEL_BUILD}")
    endif()
    add_definitions(/wd4355 /wd4251 /wd4180 /wd4996 /bigobj /Oi ${PARALLEL_FLAG} /DBOOST_ALL_NO_LIB)
    #list(APPEND OROCOS-RTT_LIBRARIES kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib  ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib Ws2_32.lib winmm.lib)
    set(RTT_HINTS HINTS "c:/Program Files/orocos") # Look here if user forgot to set CMAKE_PREFIX_PATH
  endif()
  set(CMAKE_DEBUG_POSTFIX "d")
endif(OROCOS_TARGET STREQUAL "win32")

###################################################
#                                                 #
#       Do some global set-up & checking          #
#                                                 #
###################################################

# Modules path (for searching FindXXX.cmake files)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/config")

#
# This is the first in the include path.
INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR} ${PROJ_BINARY_DIR}/ocl )

# Put your 'flags' inhere if you want them to apear in the
# orocos-components.pc file. Various .cmake scripts below do
# this.
SET( ENV{OROCOS_COMPONENTS_CFLAGS} "")
SET( ENV{OROCOS_COMPONENTS_LINKFLAGS} "")
SET( ENV{OROCOS_COMPONENTS_REQUIRES} "")

INCLUDE(${PROJ_SOURCE_DIR}/config/component_rules.cmake)
INCLUDE(${PROJ_SOURCE_DIR}/config/TestUsesFile.cmake)

# Tell UseOrocos macros we don't want to build ros-style:
set(NO_ROS_PACKAGE TRUE)


#############################
#			    #
# Building Documentation    #
#                           #
#############################

# Used by Doxygen.in !
IF(NOT RTT_VVERSION)
  SET( RTT_VVERSION "v${OCL_VERSION_MAJOR}.${OCL_VERSION_MINOR}.x" )
ENDIF(NOT RTT_VVERSION)

CONFIGURE_FILE(Doxyfile.in Doxyfile @ONLY)
ADD_CUSTOM_TARGET(docapi "doxygen" "Doxyfile")
ADD_SUBDIRECTORY( doc )

add_custom_target(luadocapi
  "luadoc" "--nofiles" "-d" "${PROJECT_BINARY_DIR}/doc/luaapi/" "."
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/lua/docapi/")

if (DOC_ONLY)
  return()
endif()

## Find Orocos RTT with same version as OCL
find_package(OROCOS-RTT ${OCL_VERSION} REQUIRED ${RTT_HINTS})
if (NOT OROCOS-RTT_FOUND)
  message(FATAL_ERROR "\n   RTT not found:\n * Is the version correct (${OCL_VERSION} or higher) ?\n * Did you build & install it ?\n * Did you source env.sh ?\n")
else()
  include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake)
  add_definitions( -DRTT_COMPONENT )
  configure_file(Doxyfile.in Doxyfile @ONLY) # Again, now with OROCOS-RTT_PATH set.
endif()

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

OPTION( BUILD_RTALLOC "Build with rt-malloc support (requires rt-malloc in RTT to be enabled)." TRUE)
# RTALLOC is forced to off on win32
IF(WIN32)
  SET(BUILD_RTALLOC OFF CACHE BOOL "docstring" FORCE)
ENDIF()



###########################################################
#                                                         #
# Look for dependencies required by individual components #
#                                                         #
###########################################################

INCLUDE(${PROJ_SOURCE_DIR}/config/check_depend.cmake)

#First unset the LOCAL_LIBRARIES
SET(ENV{SELECTED_LIBS} "")
SET(ENV{SELECTED_DIRS} "")
ADD_SUBDIRECTORY( ocl )
ADD_SUBDIRECTORY( taskbrowser )
ADD_SUBDIRECTORY( timer )
ADD_SUBDIRECTORY( helloworld )
ADD_SUBDIRECTORY( reporting )
ADD_SUBDIRECTORY( deployment )
ADD_SUBDIRECTORY( bin )
ADD_SUBDIRECTORY( logging )
ADD_SUBDIRECTORY( lua )
ADD_SUBDIRECTORY( scripts/pkg )

MESSAGE("Selected Libraries: $ENV{SELECTED_LIBS}")
SET(SELECTED_LIBS $ENV{SELECTED_LIBS})
SET(OROCOS_COMPONENTS_LINKFLAGS "$ENV{OROCOS_COMPONENTS_LINKFLAGS}")
SET(OROCOS_COMPONENTS_CFLAGS "$ENV{OROCOS_COMPONENTS_CFLAGS}")
SET(OROCOS_COMPONENTS_REQUIRES "$ENV{OROCOS_COMPONENTS_REQUIRES}")

CONFIGURE_FILE( ${PROJ_SOURCE_DIR}/ocl/ocl-config.h.in ${PROJ_SOURCE_DIR}/ocl/ocl-config.h @ONLY)
INSTALL( FILES ${PROJ_SOURCE_DIR}/ocl/ocl-config.h DESTINATION include/orocos/ocl )

###############################
#                             #
# Package configuration files #
#                             #
###############################

set(CONFIG_FILE_PATH "lib/cmake/orocos-ocl") # Folder lib/cmake is searched by default in CMake >= 2.6.3

# Main package configuration file 
INSTALL(
  FILES ${PROJ_SOURCE_DIR}/orocos-ocl-config.cmake 
  DESTINATION "${CONFIG_FILE_PATH}")

# Version file
CONFIGURE_FILE(orocos-ocl-config-version.cmake.in
               orocos-ocl-config-version.cmake @ONLY)
INSTALL(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/orocos-ocl-config-version.cmake 
  DESTINATION "${CONFIG_FILE_PATH}")

#MESSAGE("Selected dirs: $ENV{SELECTED_DIRS}")
#SET(SELECTED_DIRS $ENV{SELECTED_DIRS})

## Debugging
OPTION( DISPLAY_FLAGS "Display cmake variables at end of configure step." OFF)
MARK_AS_ADVANCED(DISPLAY_FLAGS)
IF( DISPLAY_FLAGS )
INCLUDE(config/cmake_logging.cmake)
ENDIF( DISPLAY_FLAGS )

INSTALL(FILES package.xml DESTINATION share/ocl)

# Install an env-hook if catkin is found
find_package(catkin QUIET)
if(catkin_FOUND)
  catkin_add_env_hooks(00.ocl-lua SHELLS sh DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/env-hooks)
endif()
