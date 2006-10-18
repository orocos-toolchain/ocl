# Locate GCR install directory

# This module defines
# BLF_INSTALL where to find include, lib, bin, etc.
# BLF_FOUND, is set to true

INCLUDE (${PROJ_SOURCE_DIR}/config/FindPkgConfig.cmake)

IF ( CMAKE_PKGCONFIG_EXECUTABLE )

    MESSAGE( "Using pkgconfig" )
    SET(ENV{PKG_CONFIG_PATH} "${GCR_INSTALL}/lib/pkgconfig")
    MESSAGE( "Setting environment of PKG_CONFIG_PATH to: $ENV{PKG_CONFIG_PATH}")
    PKGCONFIG( "gcr >= 1.0" GCR_FOUND GCR_INCLUDE_DIRS GCR_DEFINES GCR_LINK_DIRS GCR_LIBS )

    IF( GCR_FOUND )
        MESSAGE("   Includes in: ${GCR_INCLUDE_DIRS}")
        MESSAGE("   Libraries in: ${GCR_LINK_DIRS}")
        MESSAGE("   Libraries: ${GCR_LIBS}")
        MESSAGE("   Defines: ${GCR_DEFINES}")

	INCLUDE_DIRECTORIES( ${GCR_INCLUDE_DIRS} )
    LINK_DIRECTORIES( ${GCR_LINK_DIRS} )

    ENDIF ( GCR_FOUND )

ELSE  ( CMAKE_PKGCONFIG_EXECUTABLE )

    # Can't find pkg-config -- have to search manually
    MESSAGE( FATAL_ERROR "Can't find GCR")

ENDIF ( CMAKE_PKGCONFIG_EXECUTABLE )
