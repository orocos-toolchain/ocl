################################################################################
#
# CMake script for finding Log4cpp.
# The default CMake search process is used to locate files.
#
# This script creates the following variables:
#  LOG4CPP_FOUND: Boolean that indicates if the package was found
#  LOG4CPP_INCLUDE_DIRS: Paths to the necessary header files
#  LOG4CPP_LIBRARIES: Package libraries
#  LOG4CPP_LIBRARY_DIRS: Path to package libraries
#
################################################################################

include(FindPackageHandleStandardArgs)

# See if LOG4CPP_ROOT is not already set in CMake
IF (NOT LOG4CPP_ROOT)
    # See if LOG4CPP_ROOT is set in process environment
    IF ( NOT $ENV{LOG4CPP_ROOT} STREQUAL "" )
      SET (LOG4CPP_ROOT "$ENV{LOG4CPP_ROOT}")
      MESSAGE("Detected LOG4CPP_ROOT set to '${LOG4CPP_ROOT}'")
    ELSE()
      SET (LOG4CPP_ROOT "${CMAKE_INSTALL_PREFIX}")
      message("Setting LOG4CPP_ROOT to ${CMAKE_INSTALL_PREFIX}")
    ENDIF ()
ENDIF ()

# If LOG4CPP_ROOT is available, set up our hints
IF (LOG4CPP_ROOT)
    SET (LOG4CPP_INCLUDE_HINTS HINTS "${LOG4CPP_ROOT}/include" "${LOG4CPP_ROOT}")
    SET (LOG4CPP_LIBRARY_HINTS HINTS "${LOG4CPP_ROOT}/lib")
ENDIF ()

# Find headers and libraries
find_path(LOG4CPP_INCLUDE_DIR NAMES log4cpp/Category.hh ${LOG4CPP_INCLUDE_HINTS})
find_library(LOG4CPP_LIBRARY NAMES log4cpp ${LOG4CPP_LIBRARY_HINTS})
find_library(LOG4CPPD_LIBRARY NAMES log4cpp${CMAKE_DEBUG_POSTFIX} ${LOG4CPP_LIBRARY_HINTS})

set(LOG4CPP_VERSION 6.0.0)
message("Log4cpp version to look for: ${LOG4CPP_VERSION} (hard-coded in FindLog4cpp.cmake).")

# Determine name of Orocos' version of the log4cpp library, and see if it exists
# Linux has    liblog4cpp.so.6.0.0
# Mac OS X has liblog4cpp.6.0.0.dylib
IF (APPLE)
  GET_FILENAME_COMPONENT(LOG4CPP_OROCOS_LIB_PATH "${LOG4CPP_LIBRARY}" PATH)
  GET_FILENAME_COMPONENT(LOG4CPP_OROCOS_LIB_NAME_WE "${LOG4CPP_LIBRARY}" NAME_WE)
  GET_FILENAME_COMPONENT(LOG4CPP_OROCOS_LIB_EXT "${LOG4CPP_LIBRARY}" EXT)
  SET(LOG4CPP_OROCOS_LIB "${LOG4CPP_OROCOS_LIB_PATH}/${LOG4CPP_OROCOS_LIB_NAME_WE}.${LOG4CPP_VERSION}${LOG4CPP_OROCOS_LIB_EXT}")
ELSE (APPLE)
  SET(LOG4CPP_OROCOS_LIB "${LOG4CPP_LIBRARY}.${LOG4CPP_VERSION}")
ENDIF (APPLE)
if (LOG4CPP_LIBRARY AND NOT EXISTS ${LOG4CPP_OROCOS_LIB} )
  message("File  ${LOG4CPP_OROCOS_LIB} does not exist. Are you using the Orocos' version of log4cpp ?")
  message("  Use CMAKE_PREFIX_PATH to point to the install directory of the Orocos maintained log4cpp library.")
endif()

# Set LOG4CPP_FOUND honoring the QUIET and REQUIRED arguments
find_package_handle_standard_args(LOG4CPP DEFAULT_MSG LOG4CPP_LIBRARY LOG4CPP_INCLUDE_DIR)

# Output variables
if(LOG4CPP_FOUND)
  # Include dirs
  set(LOG4CPP_INCLUDE_DIRS ${LOG4CPP_INCLUDE_DIR})

  # Libraries
  if(LOG4CPP_LIBRARY)
    set(LOG4CPP_LIBRARIES optimized ${LOG4CPP_LIBRARY})
  else(LOG4CPP_LIBRARY)
    set(LOG4CPP_LIBRARIES "")
  endif(LOG4CPP_LIBRARY)
  if(LOG4CPPD_LIBRARY)
    set(LOG4CPP_LIBRARIES debug ${LOG4CPPD_LIBRARY} ${LOG4CPP_LIBRARIES})
  endif(LOG4CPPD_LIBRARY)

  # Link dirs
  get_filename_component(LOG4CPP_LIBRARY_DIRS ${LOG4CPP_LIBRARY} PATH)
endif()

# Advanced options for not cluttering the cmake UIs
mark_as_advanced(LOG4CPP_INCLUDE_DIR LOG4CPP_LIBRARY)
