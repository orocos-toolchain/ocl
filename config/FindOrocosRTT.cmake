# Locate Orocos::RTT install directory

# This module defines
# OROCOS_RTT_HOME where to find include, lib, bin, etc.
# OROCOS_RTT_FOUND, is set to true

INCLUDE (${PROJ_SOURCE_DIR}/config/FindPkgConfig.cmake)

IF ( CMAKE_PKGCONFIG_EXECUTABLE AND NOT SKIP_BUILD)

    MESSAGE( STATUS "Using pkgconfig" )
    
    SET(ENV{PKG_CONFIG_PATH} "${OROCOS_INSTALL}/lib/pkgconfig:${OROCOS_INSTALL}/packages/install/lib/pkgconfig/")
    #MESSAGE( "Setting environment of PKG_CONFIG_PATH to: $ENV{PKG_CONFIG_PATH}")
    MESSAGE( STATUS "Searching in ${OROCOS_INSTALL}:" )
    PKGCONFIG( "orocos-rtt >= 0.25" OROCOS_RTT OROCOS_RTT_INCLUDE_DIRS OROCOS_RTT_DEFINES OROCOS_RTT_LINK_DIRS OROCOS_RTT_LIBS )

    IF( OROCOS_RTT )
        MESSAGE("   Includes in: ${OROCOS_RTT_INCLUDE_DIRS}")
        MESSAGE("   Libraries in: ${OROCOS_RTT_LINK_DIRS}")
        MESSAGE("   Libraries: ${OROCOS_RTT_LIBS}")
        MESSAGE("   Defines: ${OROCOS_RTT_DEFINES}")

	INCLUDE_DIRECTORIES( ${OROCOS_RTT_INCLUDE_DIRS} )
        LINK_DIRECTORIES( ${OROCOS_RTT_LINK_DIRS} )

	# Detect OS:
	SET( CMAKE_REQUIRED_INCLUDES ${OROCOS_RTT_INCLUDE_DIRS})
	CHECK_INCLUDE_FILE( rtt/os/gnulinux.h OS_GNULINUX)
        IF(OS_GNULINUX)
        MESSAGE( STATUS "Detected GNU/Linux installation." )
        ENDIF(OS_GNULINUX)

	SET( CMAKE_REQUIRED_INCLUDES ${OROCOS_RTT_INCLUDE_DIRS})
	CHECK_INCLUDE_FILE( rtt/os/xenomai.h OS_XENOMAI)
        IF(OS_XENOMAI)
        MESSAGE( STATUS "Detected Xenomai installation." )
        ENDIF(OS_XENOMAI)

	SET( CMAKE_REQUIRED_INCLUDES ${OROCOS_RTT_INCLUDE_DIRS})
	CHECK_INCLUDE_FILE( rtt/os/lxrt.h OS_LXRT)
        IF(OS_LXRT)
        MESSAGE( STATUS "Detected LXRT installation." )
        ENDIF(OS_LXRT)

    ELSE  ( OROCOS_RTT )
        MESSAGE( FATAL_ERROR "Can't find Orocos Real-Time Toolkit (orocos-rtt.pc)")
    ENDIF ( OROCOS_RTT )

ELSE  ( CMAKE_PKGCONFIG_EXECUTABLE  AND NOT SKIP_BUILD)

    IF (NOT CMAKE_PKGCONFIG_EXECUTABLE)
    # Can't find pkg-config -- have to search manually
    MESSAGE( FATAL_ERROR "Can't find pkg-config ")
    ENDIF (NOT CMAKE_PKGCONFIG_EXECUTABLE)

ENDIF ( CMAKE_PKGCONFIG_EXECUTABLE  AND NOT SKIP_BUILD)
