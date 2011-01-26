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
	MESSAGE(STATUS "Detected LOG4CPP_ROOT set to '${LOG4CPP_ROOT}'")
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

set(LOG4CPP_VERSION 6.0.0)
message("Log4cpp version to look for: ${LOG4CPP_VERSION} (hard-coded in FindLog4cpp.cmake).")
if (LOG4CPP_LIBRARY AND NOT EXISTS ${LOG4CPP_LIBRARY}.${LOG4CPP_VERSION} )
  message("File  ${LOG4CPP_LIBRARY}.${LOG4CPP_VERSION} does not exist. Are you using the Orocos' version of log4cpp ?")
  message("  Use CMAKE_PREFIX_PATH to point to the install directory of the Orocos maintained log4cpp library.")
endif()

# Set LOG4CPP_FOUND honoring the QUIET and REQUIRED arguments
find_package_handle_standard_args(LOG4CPP DEFAULT_MSG LOG4CPP_LIBRARY LOG4CPP_INCLUDE_DIR)

# Output variables
if(LOG4CPP_FOUND)
  # Include dirs
  set(LOG4CPP_INCLUDE_DIRS ${LOG4CPP_INCLUDE_DIR})

  # Libraries
  set(LOG4CPP_LIBRARIES ${LOG4CPP_LIBRARY})

  # Link dirs
  get_filename_component(LOG4CPP_LIBRARY_DIRS ${LOG4CPP_LIBRARY} PATH)
endif()

# Advanced options for not cluttering the cmake UIs
mark_as_advanced(LOG4CPP_INCLUDE_DIR LOG4CPP_LIBRARY)
