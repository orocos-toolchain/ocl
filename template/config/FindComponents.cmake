# Locate COMPONENTS install directory

# This module defines
# BLF_INSTALL where to find include, lib, bin, etc.
# BLF_FOUND, is set to true

INCLUDE (${PROJ_SOURCE_DIR}/config/FindPkgConfig.cmake)

IF ( CMAKE_PKGCONFIG_EXECUTABLE )

    MESSAGE( "Using pkgconfig" )
    SET(ENV{PKG_CONFIG_PATH} "${COMPONENTS_INSTALL}/lib/pkgconfig")
    MESSAGE( "Setting environment of PKG_CONFIG_PATH to: $ENV{PKG_CONFIG_PATH}")
    PKGCONFIG( "orocos-components >= 0.1.0" COMPONENTS_FOUND COMPONENTS_INCLUDE_DIRS COMPONENTS_DEFINES COMPONENTS_LINK_DIRS COMPONENTS_LIBS )

    IF( COMPONENTS_FOUND )
        MESSAGE("   Includes in: ${COMPONENTS_INCLUDE_DIRS}")
        MESSAGE("   Libraries in: ${COMPONENTS_LINK_DIRS}")
        MESSAGE("   Libraries: ${COMPONENTS_LIBS}")
        MESSAGE("   Defines: ${COMPONENTS_DEFINES}")

	INCLUDE_DIRECTORIES( ${COMPONENTS_INCLUDE_DIRS} )
    LINK_DIRECTORIES( ${COMPONENTS_LINK_DIRS} )

    ENDIF ( COMPONENTS_FOUND )

ELSE  ( CMAKE_PKGCONFIG_EXECUTABLE )

    # Can't find pkg-config -- have to search manually
    MESSAGE( FATAL_ERROR "Can't find COMPONENTS")

ENDIF ( CMAKE_PKGCONFIG_EXECUTABLE )
