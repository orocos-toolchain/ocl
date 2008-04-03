# Locate COMEDI install directory

# This module defines
# COMEDI_INSTALL where to find include, lib, bin, etc.
# COMEDILIB_FOUND, is set to true

INCLUDE (${PROJ_SOURCE_DIR}/config/FindPkgConfig.cmake)
INCLUDE (${PROJ_SOURCE_DIR}/config/component_rules.cmake)

# GNU/Linux detection of Comedi lib uses pkgconfig or plain header search.
IF (OS_GNULINUX OR OS_XENOMAI)
  IF ( CMAKE_PKGCONFIG_EXECUTABLE )

      SET(ENV{PKG_CONFIG_PATH} "${COMEDI_INSTALL}/lib/pkgconfig/")
      MESSAGE( "Looking for comedilib headers in ${COMEDI_INSTALL}/include ")
      PKGCONFIG( "comedilib >= 0.7.0" COMEDILIB_FOUND COMEDI_INCLUDE_DIRS COMEDI_DEFINES COMEDI_LINK_DIRS COMEDI_LIBS )

  ENDIF( CMAKE_PKGCONFIG_EXECUTABLE )
  IF ( NOT COMEDILIB_FOUND )
        MESSAGE("Looking for comedilib headers in ${COMEDI_INSTALL}/include")
	SET(COMEDILIB_FOUND COMEDILIB_FOUND-NOTFOUND)
	FIND_FILE("comedilib.h" COMEDILIB_FOUND ${COMEDI_INSTALL}/include)
	SET(COMEDI_INCLUDE_DIRS INTERNAL "-I${COMEDI_INSTALL}/include")
	SET(COMEDI_LINK_DIRS INTERNAL "-I${COMEDI_INSTALL}/lib")
	SET(COMEDI_LIBS INTERNAL "comedilib")
  ENDIF ( NOT COMEDILIB_FOUND )

  IF( COMEDILIB_FOUND )
        MESSAGE("   Comedi Lib found.")
        MESSAGE("   Includes in: '${COMEDI_INCLUDE_DIRS}'")
        MESSAGE("   Libraries in: '${COMEDI_LINK_DIRS}'")
        MESSAGE("   Libraries: '${COMEDI_LIBS}'")
        MESSAGE("   Defines: '${COMEDI_DEFINES}'")

	OROCOS_PKGCONFIG_INCPATH("")
	OROCOS_PKGCONFIG_INCPATH("${COMEDI_INCLUDE_DIRS}")
	OROCOS_PKGCONFIG_LIBPATH("${COMEDI_INSTALL}/lib")
        OROCOS_PKGCONFIG_LIBS( comedi )
	#INCLUDE_DIRECTORIES( ${COMEDI_INCLUDE_DIRS} )
	LINK_DIRECTORIES( ${COMEDI_LINK_DIRS} )
        LINK_LIBRARIES( ${COMEDI_LIBS} )
	SET(COMEDI_FOUND INTERNAL TRUE)
  ENDIF( COMEDILIB_FOUND )
ENDIF ( OS_GNULINUX OR OS_XENOMAI )

# For LXRT, do a manual search.   
    IF (OS_LXRT)
        # Try comedi/lxrt
        MESSAGE("Looking for comedi/LXRT headers in ${COMEDI_INSTALL}/include")
        SET(COMEDI_INCLUDE_DIR COMEDI_INCLUDE_DIR-NOTFOUND)
	FIND_PATH(COMEDI_INCLUDE_DIR rtai_comedi.h "${COMEDI_INSTALL}/include")
	#FIND_LIBRARY(COMEDI_LIBRARY NAMES kcomedilxrt PATH "/usr/realtime/lib") 
	IF ( COMEDI_INCLUDE_DIR )
	  # Add comedi header path and lxrt comedi lib 
	  #INCLUDE_DIRECTORIES( ${COMEDI_INSTALL}/include )
	  SET(COMEDI_INCLUDE_DIRS ${COMEDI_INSTALL}/include )
	  #LINK_LIBRARIES( kcomedilxrt )
	  #OROCOS_PKGCONFIG_INCPATH("${COMEDI_INSTALL}/include")
	  #OROCOS_PKGCONFIG_LIBS("-lkcomedilxrt")
	  #OROCOS_PKGCONFIG_LIBPATH("{COMEDI_INSTALL}/lib")
	  MESSAGE("rtai_comedi.h found.")
          SET(COMEDI_FOUND INTERNAL TRUE)
	ELSE(COMEDI_INCLUDE_DIR )
	  MESSAGE("rtai_comedi.h not found.")
	ENDIF ( COMEDI_INCLUDE_DIR )
	SET(COMEDI_INCLUDE_DIR INTERNAL)
    ENDIF (OS_LXRT)

