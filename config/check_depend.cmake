###########################################################
#                                                         #
# Look for dependencies required by individual components #
#                                                         #
###########################################################

# Can we use pkg-config?
INCLUDE (${PROJ_SOURCE_DIR}/config/FindPkgConfig.cmake)

# For examples of dependency checks see orca-components/config/check_depend.cmake

# Look for Player (ctually looks for v.>=1.6)
# INCLUDE (${PROJ_SOURCE_DIR}/config/FindPlayer2.cmake)

# Look for libftd2xx.so (a high level USB library)
CHECK_INCLUDE_FILE_CXX( ftd2xx.h FTD2XX )
IF ( FTD2XX )
    MESSAGE("-- Looking for libftd2xx - found")
ELSE ( FTD2XX )
    MESSAGE("-- Looking for libftd2xx - not found")
ENDIF ( FTD2XX )

# Look for canlib.h
CHECK_INCLUDE_FILE_CXX( canlib.h CANLIB )
IF ( CANLIB )
    MESSAGE("-- Looking for canlib - found")
ELSE ( CANLIB )
    MESSAGE("-- Looking for canlib - not found")
ENDIF ( CANLIB )

# Check for Qt
INCLUDE (${CMAKE_ROOT}/Modules/FindQt4.cmake)
# we do NOT want 4.0.x
IF ( QTVERSION MATCHES "4.0.*")
    SET ( QT4 FALSE )
ENDIF ( QTVERSION MATCHES "4.0.*")
IF( QT4 )
    MESSAGE("-- Looking for Qt4 >= 4.1 - found")
    MESSAGE("   version: ${QTVERSION}" )
    MESSAGE("   Core library: ${QT_QTCORE_LIBRARY}" )
    MESSAGE("   GUI library: ${QT_QTGUI_LIBRARY}" )
    MESSAGE("   Includes in ${QT_INCLUDE_DIR}")
    MESSAGE("   Libraries in ${QT_LIBRARY_DIR}")
    MESSAGE("   Libraries ${QT_LIBRARIES}" )
ELSE ( QT4 )
    MESSAGE("-- Looking for Qt4 >= 4.1 - not found")
ENDIF ( QT4 )

# Check for OpenCV-0.9.7
INCLUDE (${PROJ_SOURCE_DIR}/config/FindOpenCV.cmake)
IF ( OPENCV_FOUND )
    MESSAGE("-- Looking for OpenCV - found")
ELSE ( OPENCV_FOUND )
    MESSAGE("-- Looking for OpenCV - not found")
ENDIF ( OPENCV_FOUND )

# Look for firewire headers (for firewire cameras)
CHECK_INCLUDE_FILE_CXX( libdc1394/dc1394_control.h 1394 )

# Look for video-for-linux (for usb cameras).
CHECK_INCLUDE_FILE( linux/videodev.h V4L )
CHECK_INCLUDE_FILE( linux/videodev2.h V4L2 )

INCLUDE( ${CMAKE_ROOT}/Modules/FindCurses.cmake )
IF ( CURSES_INCLUDE_DIR )
    MESSAGE("-- Looking for libncurses - found")
    SET( CURSES 1 CACHE INTERNAL "libncurses" )
ELSE ( CURSES_INCLUDE_DIR )
    MESSAGE("-- Looking for libncurses - not found")
    SET( CURSES 0 CACHE INTERNAL "libncurses" )
ENDIF ( CURSES_INCLUDE_DIR )

# Check for ZLIB
INCLUDE (${CMAKE_ROOT}/Modules/FindZLIB.cmake)
SET( HAVE_ZLIB ${ZLIB} )

# Look for boost
CHECK_INCLUDE_FILE_CXX( boost/numeric/ublas/matrix.hpp BOOST )
IF ( BOOST )
    MESSAGE("-- Looking for Boost - found")
ELSE ( BOOST )
    MESSAGE("-- Looking for Boost - not found")
ENDIF ( BOOST )

# Look for ulapack
CHECK_INCLUDE_FILE_CXX( ulapack/eig.hpp ULAPACK )


