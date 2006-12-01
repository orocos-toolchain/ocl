# Locate KDL install directory

# This module defines
# KDL_INSTALL where to find include, lib, bin, etc.
# KDL_FOUND, is set to true

INCLUDE (${PROJ_SOURCE_DIR}/config/FindPkgConfig.cmake)

IF ( CMAKE_PKGCONFIG_EXECUTABLE )

    MESSAGE( STATUS "Detecting KDL" )
    
    SET(ENV{PKG_CONFIG_PATH} "${KDL_INSTALL}/lib/pkgconfig/")
    MESSAGE( "Looking for KDL in: ${KDL_INSTALL}")
    PKGCONFIG( "orocos-kdl >= 0.2" KDL_FOUND KDL_INCLUDE_DIRS KDL_DEFINES KDL_LINK_DIRS KDL_LIBS )

    IF( KDL_FOUND )
        MESSAGE("   Includes in: ${KDL_INCLUDE_DIRS}")
        MESSAGE("   Libraries in: ${KDL_LINK_DIRS}")
        MESSAGE("   Libraries: ${KDL_LIBS}")
        MESSAGE("   Defines: ${KDL_DEFINES}")

	MESSAGE( "Looking for KDL Toolkit in: ${KDL_INSTALL}")
	SET(ENV{PKG_CONFIG_PATH} "${KDL_INSTALL}/lib/pkgconfig/:${OROCOS_INSTALL}/lib/pkgconfig")
	PKGCONFIG( "kdltk >= 0.2" KDLTK_FOUND KDLTK_INCLUDE_DIRS KDLTK_DEFINES KDLTK_LINK_DIRS KDLTK_LIBS )
	IF(KDLTK_FOUND)
	  INCLUDE_DIRECTORIES( ${KDL_INCLUDE_DIRS} ${KDLTK_INCLUDE_DIRS} )
	  LINK_DIRECTORIES( ${KDL_LINK_DIRS} ${KDLTK_LINK_DIRS})
	ENDIF ( KDLTK_FOUND )
      ENDIF ( KDL_FOUND )

ELSE  ( CMAKE_PKGCONFIG_EXECUTABLE )

    # Can't find pkg-config -- have to search manually
    MESSAGE( FATAL_ERROR "Can't find KDL without pkgconfig !")

ENDIF ( CMAKE_PKGCONFIG_EXECUTABLE )
