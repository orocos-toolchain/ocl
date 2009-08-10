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
FIND_PATH( FTD2XX ftd2xx.h )
IF ( FTD2XX )
    MESSAGE("-- Looking for libftd2xx - found")
ELSE ( FTD2XX )
    MESSAGE("-- Looking for libftd2xx - not found")
ENDIF ( FTD2XX )

# Look for canlib.h
FIND_PATH( CANLIB canlib.h )
IF ( CANLIB )
    MESSAGE("-- Looking for canlib - found")
ELSE ( CANLIB )
    MESSAGE("-- Looking for canlib - not found")
ENDIF ( CANLIB )

# Look for can.h
FIND_PATH( SOCKETLIB linux/can.h )
IF ( SOCKETLIB )
    MESSAGE("-- Looking for can.h - found")
    INCLUDE_DIRECTORIES(${SOCKETLIB})
ELSE ( SOCKETLIB )
    MESSAGE("-- Looking for can.h - not found")
ENDIF ( SOCKETLIB )

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
FIND_PATH( 1394 libdc1394/dc1394_control.h )

# Look for video-for-linux (for usb cameras).
FIND_PATH( V4L linux/videodev.h )
FIND_PATH( V4L2 linux/videodev2.h )

INCLUDE( ${CMAKE_ROOT}/Modules/FindCurses.cmake )
IF ( CURSES_INCLUDE_DIR )
    MESSAGE("-- Looking for libncurses - found")
    SET( CURSES 1 CACHE INTERNAL "libncurses" )
ELSE ( CURSES_INCLUDE_DIR )
    MESSAGE("-- Looking for libncurses - not found")
    SET( CURSES 0 CACHE INTERNAL "libncurses" )
ENDIF ( CURSES_INCLUDE_DIR )

FIND_PATH( READLINE_H readline/readline.h )
IF ( READLINE_H )
    MESSAGE("-- Looking for readline/readline.h - found")
    FIND_LIBRARY(READLINE_LIBRARY readline )
    SET( READLINE 1 CACHE INTERNAL "libreadline" )
ELSE ( READLINE_H  )
    MESSAGE("-- Looking for readline/readline.h - not found")
    SET( READLINE 0 CACHE INTERNAL "libreadline" )
ENDIF ( READLINE_H )

# Check for ZLIB
INCLUDE (${CMAKE_ROOT}/Modules/FindZLIB.cmake)
SET( HAVE_ZLIB ${ZLIB} )

# Look for boost
FIND_PATH(BOOST boost/numeric/ublas/matrix.hpp  )
IF ( BOOST )
    MESSAGE("-- Looking for Boost Ublas - found")
ELSE ( BOOST )
    MESSAGE("-- Looking for Boost Ublas - not found")
ENDIF ( BOOST )
FIND_PATH(BOOST_OPTIONS boost/program_options.hpp )
IF ( BOOST_OPTIONS )
    MESSAGE("-- Looking for boost::program_options - found")

	# now find the location of the associated library
    FIND_LIBRARY(BOOST_OPTIONS_LIBRARY boost_program_options NAMES boost_program_options boost_program_options-mt)
	IF ( BOOST_OPTIONS_LIBRARY )
      MESSAGE("-- Looking for boost::program_options library - found")
	  GET_FILENAME_COMPONENT(BOOST_OPTIONS_LIBRARY_DIR ${BOOST_OPTIONS_LIBRARY} PATH)
	  LINK_DIRECTORIES(${BOOST_OPTIONS_LIBRARY_DIR})
	ELSE ( BOOST_OPTIONS_LIBRARY )
      MESSAGE(FATAL_ERROR "-- Looking for boost::program_options library - not found. \n ==> Please install libboost-program-options-dev or similar. Then remove CMakeCache.txt and re-run cmake." )
	ENDIF ( BOOST_OPTIONS_LIBRARY )

ELSE ( BOOST_OPTIONS )
    MESSAGE(FATAL_ERROR "-- Looking for boost::program_options - not found. \n ==> Please install libboost-program-options-dev or similar. Then remove CMakeCache.txt and re-run cmake." )
ENDIF ( BOOST_OPTIONS )

# Look for ulapack
FIND_PATH( ULAPACK ulapack/eig.hpp )


