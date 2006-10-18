# Locate OpenCv-0.9.7 install directory

# This module defines
# OPENCV7_HOME where to find include, lib, bin, etc.
# OPENCV7_FOUND, If false, don't try to use Ice.

INCLUDE (${PROJ_SOURCE_DIR}/config/FindPkgConfig.cmake)

IF ( CMAKE_PKGCONFIG_EXECUTABLE )

    MESSAGE( "Using pkgconfig" )
    
    # Find all the opencv stuff with pkg-config
    PKGCONFIG( "opencv >= 0.9.7" OPENCV7_FOUND OPENCV7_INCLUDE_DIRS OPENCV7_DEFINES OPENCV7_LINK_DIRS OPENCV7_LIBS )

    IF( OPENCV7_FOUND )
        MESSAGE("   Includes in: ${OPENCV7_INCLUDE_DIRS}")
        MESSAGE("   Libraries in: ${OPENCV7_LINK_DIRS}")
        MESSAGE("   Libraries: ${OPENCV7_LIBS}")
        MESSAGE("   Defines: ${OPENCV7_DEFINES}")
    ENDIF ( OPENCV7_FOUND )

ELSE  ( CMAKE_PKGCONFIG_EXECUTABLE )

    # Can't find pkg-config -- have to search manually

ENDIF ( CMAKE_PKGCONFIG_EXECUTABLE )
