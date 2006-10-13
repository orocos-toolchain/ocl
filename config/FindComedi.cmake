# Locate COMEDI install directory

# This module defines
# COMEDI_INSTALL where to find include, lib, bin, etc.
# COMEDILIB_FOUND, is set to true

INCLUDE (${PROJ_SOURCE_DIR}/config/FindPkgConfig.cmake)
INCLUDE (${PROJ_SOURCE_DIR}/config/component_rules.cmake)

# GNU/Linux detection of Comedi lib uses pkgconfig or plain header search.
IF (OROCOS_TARGET STREQUAL "gnulinux")
  IF ( CMAKE_PKGCONFIG_EXECUTABLE )

      SET(ENV{PKG_CONFIG_PATH} "${COMEDI_INSTALL}/lib/pkgconfig/")
      MESSAGE( "Looking for comedilib headers in ${COMEDI_INSTALL}/include ")
      PKGCONFIG( "comedilib >= 0.7.0" COMEDILIB_FOUND COMEDI_INCLUDE_DIRS COMEDI_DEFINES COMEDI_LINK_DIRS COMEDI_LIBS )

  ENDIF( CMAKE_PKGCONFIG_EXECUTABLE )
  IF ( NOT COMEDILIB_FOUND )
        MESSAGE("Looking for comedilib headers in ${COMEDI_INSTALL}/include")
	SET(CMAKE_REQUIRED_INCLUDES ${COMEDI_INSTALL}/include )
	CHECK_INCLUDE_FILE("comedilib.h" COMEDILIB_FOUND)
	SET(COMEDI_INCLUDE_DIRS INTERNAL "-I ${COMEDI_INSTALL}/include")
	SET(COMEDI_LINK_DIRS INTERNAL "-I ${COMEDI_INSTALL}/lib")
	SET(COMEDI_LIBS INTERNAL "comedilib")

  ENDIF ( NOT COMEDILIB_FOUND )

  IF( COMEDILIB_FOUND )
        MESSAGE("   Comedi Lib found.")
        MESSAGE("   Includes in: ${COMEDI_INCLUDE_DIRS}")
        MESSAGE("   Libraries in: ${COMEDI_LINK_DIRS}")
        MESSAGE("   Libraries: ${COMEDI_LIBS}")
        MESSAGE("   Defines: ${COMEDI_DEFINES}")

        OROCOS_PKGCONFIG_LIBS("-L${COMEDI_INSTALL}/lib -lcomedilib")
	INCLUDE_DIRECTORIES( ${COMEDI_INCLUDE_DIRS} )
	LINK_DIRECTORIES( ${COMEDI_LINK_DIRS} )
        LINK_LIBRARIES( ${COMEDI_LIBS} )
  ENDIF( COMEDILIB_FOUND )
ENDIF (OROCOS_TARGET STREQUAL "gnulinux")

# For LXRT, do a manual search.   
    IF (OROCOS_TARGET STREQUAL "lxrt")
        # Try comedi/lxrt
        MESSAGE("Looking for comedi/LXRT headers in ${COMEDI_INSTALL}/include")
	FIND_PATH(COMEDI_INCLUDE_DIR linux/comedi.h "${COMEDI_INSTALL}/include")
	#FIND_LIBRARY(COMEDI_LIBRARY NAMES kcomedilxrt PATH "/usr/realtime/lib") 
	IF ( COMEDI_INCLUDE_DIR )
	  # Add comedi header path and lxrt comedi lib 
	  INCLUDE_DIRECTORIES( ${COMEDI_INSTALL}/include )
	  LINK_LIBRARIES( kcomedilxrt )
	  OROCOS_PKGCONFIG_LIBS("-L${COMEDI_INSTALL}/lib -lkcomedilxrt")
	  MESSAGE("linux/comedi.h found.")
	ELSE(COMEDI_INCLUDE_DIR )
	  MESSAGE("linux/comedi.h not found.")
	ENDIF ( COMEDI_INCLUDE_DIR )
    ENDIF (OROCOS_TARGET STREQUAL "lxrt")

