#
# Include cmake modules required to look for dependencies
#
INCLUDE( ${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake )
INCLUDE( ${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake )



# Try to find Ice
# Modified. Environment variables are discouraged. 
# If you want to manually specify the location of ICE_HOME
# do it using -DICE_HOME:STRING=<locn> on the command line
 
IF ( DEFINED ICE_HOME )
  # Ice home is specified with a command line option or it's already in cache
  MESSAGE("Ice location was specified or using cached value: ${ICE_HOME}")
ELSE ( DEFINED ICE_HOME )
    # Find Ice Installation
    # Will search several standard places starting with an env. variable ICE_HOME
    INCLUDE ( config/FindIce.cmake )
    IF( ICE_FOUND )
        MESSAGE("-- Looking for Ice - found in ${ICE_HOME} ")
    ELSE ( ICE_FOUND )
        MESSAGE( "-- Looking for Ice - not found." )
        MESSAGE( FATAL_ERROR "   Please install Ice, ** delete CMakeCache.txt **, then re-run CMake." )
    ENDIF ( ICE_FOUND )
ENDIF ( DEFINED ICE_HOME )

#There is no easy way to pass separate compile and link 
#options to the macro, so assume we are told the truth when
#on the windows platform. 
IF ( NOT WIN32 )
  CHECK_INCLUDE_FILE_CXX( "Ice/Ice.h" HAVE_ICE "-I${ICE_HOME}/include -L${ICE_HOME}/lib -lIce -lIceUtil")
ELSE (NOT WIN32)
  SET (HAVE_ICE TRUE)
ENDIF(NOT WIN32)

#IF ( NOT HAVE_ICE )
#  MESSAGE( FATAL_ERROR "Ice was found but cannot be used. Please install it, ** delete CMakeCache.txt **, then re-run CMake." )
#ENDIF ( NOT HAVE_ICE )

#
# If we're using gcc, make sure the version is OK.
#
#STRING( REGEX MATCH gcc USING_GCC ${CMAKE_C_COMPILER} )
IF (  ${CMAKE_C_COMPILER} MATCHES gcc )
  EXEC_PROGRAM( ${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE CMAKE_C_COMPILER_VERSION )
  # Why doesn't this work?
  #STRING( REGEX MATCHALL "gcc\.*" VERSION_STRING ${CMAKE_C_COMPILER} )
  IF( CMAKE_C_COMPILER_VERSION MATCHES ".*4\\.[0-9]\\.[0-9]" )
    MESSAGE("gcc version: ${CMAKE_C_COMPILER_VERSION}")
#   ELSE(CMAKE_C_COMPILER_VERSION MATCHES ".*4\\.[0-9]\\.[0-9]")
#     MESSAGE("ERROR: You seem to be using gcc version:")
#     MESSAGE("${CMAKE_C_COMPILER_VERSION}")
#     MESSAGE( FATAL_ERROR "ERROR: For gcc, Orocos requires version 4.x")
  ENDIF(CMAKE_C_COMPILER_VERSION MATCHES ".*4\\.[0-9]\\.[0-9]")
ENDIF (  ${CMAKE_C_COMPILER} MATCHES gcc )


#
# Check for Doxygen and enable documentation building
#
INCLUDE( ${CMAKE_ROOT}/Modules/FindDoxygen.cmake )
IF ( DOXYGEN )
  MESSAGE( "Found Doxygen -- documentation can be built" )

  OPTION( GENERATE_DOCUMENTATION "Build Documentation" OFF )

ELSE ( DOXYGEN )
  MESSAGE( "Doxygen not found -- unable to build documentation" )
ENDIF ( DOXYGEN )


#
# An option for tests, to make it easy to turn off all tests
#
OPTION( BUILD_TESTS "Turn me off to disable compilation of all tests" ON )

#
# An option for tests, to make it easy to turn off all tests
#
OPTION( BUILD_SANDBOX "Turn me on to enable compilation of everything in the sandbox" OFF )

#
# Project-specific global setup
#
INCLUDE( ${PROJ_SOURCE_DIR}/config/project/project_setup.cmake )
