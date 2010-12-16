# Modules path (for searching FindXXX.cmake files)
list(APPEND CMAKE_MODULE_PATH "${PROJ_SOURCE_DIR}/config")

include("CMakeDependentOption")

#
# Check for Doxygen and enable documentation building
#
find_package( Doxygen )
IF ( DOXYGEN_FOUND )
  MESSAGE( "Found Doxygen -- documentation can be built" )

ELSE ( DOXYGEN_FOUND )
  MESSAGE( "Doxygen not found -- unable to build documentation" )
ENDIF ( DOXYGEN_FOUND )

CMAKE_DEPENDENT_OPTION( DOC_GENERATE_API "Build API Documentation" OFF "DOXYGEN_FOUND" OFF )

#
# An option for tests, to make it easy to turn off all tests
#
OPTION( BUILD_TESTS "Turn me off to disable compilation of all tests" OFF )

###########################################################
#                                                         #
# Look for dependencies required by individual components #
#                                                         #
###########################################################

find_package( Curses )
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
    SET( READLINE_INCLUDE_DIR ${READLINE_H} )
ELSE ( READLINE_H  )
    MESSAGE("-- Looking for readline/readline.h - not found")
    SET( READLINE 0 CACHE INTERNAL "libreadline" )
ENDIF ( READLINE_H )

# Since in ros-builds, log4cpp will be installed in log4cpp/install,
# we look there too.
if (ROS_ROOT)
  rosbuild_find_ros_package( log4cpp )
  set(LOG4CPP_ROOT ${log4cpp_PACKAGE_PATH}/install)
  message("ROS log4cpp in ${LOG4CPP_ROOT}")
endif(ROS_ROOT)
find_package( Log4cpp REQUIRED )
if(LOG4CPP_FOUND)
  message("Found log4cpp in ${LOG4CPP_INCLUDE_DIRS}")
endif(LOG4CPP_FOUND)

find_package(Boost COMPONENTS program_options filesystem system)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} ${READLINE_INCLUDE_DIR} ${CURSES_INCLUDE_DIR} )

