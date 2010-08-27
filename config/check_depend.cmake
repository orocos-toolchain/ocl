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
    SET( READLINE_INCLUDE_DIR ${READLINE_H} )
ELSE ( READLINE_H  )
    MESSAGE("-- Looking for readline/readline.h - not found")
    SET( READLINE 0 CACHE INTERNAL "libreadline" )
ENDIF ( READLINE_H )

# Look for boost
FIND_PATH(BOOST boost/numeric/ublas/matrix.hpp  )
IF ( BOOST )
    MESSAGE("-- Looking for Boost Ublas - found")
ELSE ( BOOST )
    MESSAGE("-- Looking for Boost Ublas - not found")
ENDIF ( BOOST )

find_package(Boost COMPONENTS program_options)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} ${READLINE_INCLUDE_DIR} )

# Look for Log4cpp (if needed
IF ( BUILD_LOGGING )
  FIND_PACKAGE( Log4cpp REQUIRED )
ENDIF ( BUILD_LOGGING )
