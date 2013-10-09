########################################################################################################################
#
# CMake package configuration file for the OROCOS-RTT package.
# This script imports targets and sets up the variables needed to use the package.
# In case this file is installed in a nonstandard location, its location can be specified using the OROCOS-RTT_DIR cache
# entry.
#
# find_package COMPONENTS represent OROCOS-OCL plugins such as logging, deployment, timer, etc.
# This uses the orocos_find_package() macro defined in the RTT use-file.
#
# This script sets the following variables:
#  OROCOS-OCL_FOUND: Boolean that indicates if OROCOS-OCL was found
#  OROCOS-OCL_INCLUDE_DIRS: Paths to the necessary header files
#  OROCOS-OCL_LIBRARIES: Libraries to link against to use OROCOS-OCL and all of the requested components
#  OROCOS-OCL_DEFINITIONS: Definitions to use when compiling code that uses OROCOS-OCL
#
# This script additionally sets variables for each requested find_package COMPONENTS (OROCOS-OCL plugins).
# For example, for the ''ocl-logging'' plugin this would be:
#  OROCOS-OCL_OCL-LOGGING_FOUND: Boolean that indicates if the component was found
#  OROCOS-OCL_OCL-LOGGING_LIBRARIES: Libraries to link against to use this component (notice singular _LIBRARY suffix)
#
# Note for advanced users: Apart from the OROCOS-OCL_*_LIBRARIES variables, non-COMPONENTS targets can be accessed by
# their imported name, e.g., target_link_libraries(bar @IMPORTED_TARGET_PREFIX@orocos-ocl-gnulinux_dynamic).
# This of course requires knowing the name of the desired target, which is why using the OROCOS-OCL_*_LIBRARIES
# variables is recommended.
#
# Example usage:
#  find_package(OROCOS-OCL 2.0.5 EXACT REQUIRED ocl-logging) # Defines OROCOS-OCL_OCL-LOGGING_*
#  find_package(OROCOS-OCL QUIET COMPONENTS ocl-deployment) # Defines OROCOS-OCL_OCL-DEPLOYMENT_*
#
########################################################################################################################


########################################################################################################################
#
# Initialization
#
########################################################################################################################

# Utilities
include(FindPkgConfig)

# Find RTT to get OROCOS_TARGET etc.
if(NOT OROCOS_TARGET)
  find_package(OROCOS-RTT REQUIRED)
endif()
if(NOT COMMAND orocos_find_package)
  include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake)
endif()

# Path to current file
get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

########################################################################################################################
#
# OROCOS-OCL core
#
########################################################################################################################

# Only check for new modules if already found.
# This test is very important, otherwise running this script multiple times in the same project will try to add multiple
# imported targets with the same name, yielding a configuration-time error
if(NOT OROCOS-OCL_FOUND)

  # Use pkg-config to get ocl flags
  orocos_find_package(orocos-ocl)

  # Confirm found, not cached 
  if(orocos-ocl_FOUND)
    message(STATUS "Orocos-OCL found in ${SELF_DIR}")

    set(OROCOS-OCL_FOUND True)
    set(OROCOS-OCL_LIBRARIES ${orocos-ocl_LIBRARIES})
    set(OROCOS-OCL_INCLUDE_DIRS ${orocos-ocl_INCLUDE_DIRS})
    set(OROCOS-OCL_LIBRARY_DIRS ${orocos-ocl_LIBRARY_DIRS})
    set(OROCOS-OCL_CFLAGS_OTHER ${orocos-ocl_CFLAGS_OTHER})
    set(OROCOS-OCL_LDFLAGS_OTHER ${orocos-ocl_LDFLAGS_OTHER})
  endif()

endif()


########################################################################################################################
#
# Components: This is called each time a find_package is done:
#
########################################################################################################################

if(OROCOS-OCL_FOUND)
  # Bootstrap components on pkgconfig
  foreach(COMPONENT ${OROCOS-OCL_FIND_COMPONENTS})
    orocos_find_package(${COMPONENT} OROCOS_ONLY) 

    if(${COMPONENT}_FOUND)
      list(APPEND OROCOS-OCL_FOUND_COMPONENTS ${COMPONENT})
      list(APPEND OROCOS-OCL_LIBRARIES ${${COMPONENT}_LIBRARIES})
      list(APPEND OROCOS-OCL_INCLUDE_DIRS ${${COMPONENT}_INCLUDE_DIRS})

      string(TOUPPER ${COMPONENT} COMPONENT_UPPER)

      set(OROCOS-OCL_${COMPONENT_UPPER}_FOUND True)
      set(OROCOS-OCL_${COMPONENT_UPPER}_LIBRARIES ${${COMPONENT}_LIBRARIES})
    else()
      LIST(APPEND OROCOS-OCL_MISSING_COMPONENTS ${COMPONENT})
    endif()
  endforeach()
endif()


########################################################################################################################
#
# Print success message
#
########################################################################################################################

if(NOT OROCOS-OCL_FIND_QUIETLY)

  message(STATUS "Found orocos-ocl ${OROCOS-OCL_VERSION} for the ${OROCOS_TARGET} target.")

  # List found components
  if(OROCOS-OCL_FOUND_COMPONENTS)
    message(STATUS "- Found requested orocos-ocl components:${OROCOS-OCL_FOUND_COMPONENTS}")
  endif()

  # List missing components
  if(OROCOS-OCL_MISSING_COMPONENTS)
    message(STATUS "- Could NOT find requested orocos-ocl components:${OROCOS-OCL_MISSING_COMPONENTS}")
  endif()
endif()

