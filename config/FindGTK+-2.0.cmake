# Locate BFL install directory

# This module defines
# BLF_INSTALL where to find include, lib, bin, etc.
# BLF_FOUND, is set to true

INCLUDE (${PROJ_SOURCE_DIR}/config/FindPkgConfig.cmake)

IF ( CMAKE_PKGCONFIG_EXECUTABLE )

    MESSAGE( STATUS "Detecting GTK+-2.0" )
    PKGCONFIG( "gtk+-2.0" GTK_FOUND GTK_INCLUDE_DIRS GTK_DEFINES GTK_LINK_DIRS GTK_LIBS )

    IF( GTK_FOUND )
        MESSAGE("   Includes in: ${GTK_INCLUDE_DIRS}")
        MESSAGE("   Libraries in: ${GTK_LINK_DIRS}")
        MESSAGE("   Libraries: ${GTK_LIBS}")
        MESSAGE("   Defines: ${GTK_DEFINES}")

	INCLUDE_DIRECTORIES( ${GTK_INCLUDE_DIRS} )
    LINK_DIRECTORIES( ${GTK_LINK_DIRS} )

    ENDIF ( GTK_FOUND )

ELSE  ( CMAKE_PKGCONFIG_EXECUTABLE )

    # Can't find pkg-config -- have to search manually
    MESSAGE( FATAL_ERROR "Can't find GTK+-2.0 without pkgconfig !")

ENDIF ( CMAKE_PKGCONFIG_EXECUTABLE )
