#
# Include and link against required stuff
#

ADD_DEFINITIONS( "-Wall" )

#
# Components should add themselves by calling 'GLOBAL_ADD_COMPONENT' 
# instead of 'ADD_LIBRARY' in CMakeLists.txt.
#
# This gives a centralised location where all components are registered
# and lets us add various things to all components in just one place.
#
#
# Usage: GLOBAL_ADD_COMPONENT( COMPONENT_NAME src1 src2 src3 )
#
MACRO( GLOBAL_ADD_COMPONENT COMPONENT_NAME )
  
  IF(STANDALONE_COMPONENTS)
     MESSAGE( "BROKEN: Building Stand-alone component ${COMPONENT_NAME}" )
     ADD_EXECUTABLE( ${COMPONENT_NAME} ${ARGN} )
     INSTALL_TARGETS( /bin ${COMPONENT_NAME} )
     TARGET_LINK_LIBRARIES( ${COMPONENT_NAME} ${OROCOS_RTT_LIBS} )
  ENDIF(STANDALONE_COMPONENTS)
  
  IF(GLOBAL_LIBRARY)
     MESSAGE( "BROKEN: Adding ${COMPONENT_NAME} to global sources:[ ${GLOBAL_LIBRARY_SRCS} ]" )
     SET (GLOBAL_LIBRARY_SRCS "${GLOBAL_LIBRARY_SRCS} ${ARGN}" )
  ENDIF(GLOBAL_LIBRARY)

  IF(LOCAL_LIBRARY)
     MESSAGE( "Building Stand-alone library ${COMPONENT_NAME}" )
     ADD_LIBRARY( ${COMPONENT_NAME} STATIC ${ARGN} )
     INSTALL_TARGETS( /lib ${COMPONENT_NAME} )
     SET (LOCAL_LIBRARIES "${LOCAL_LIBRARIES} ${COMPONENT_NAME}" )
     LINK_DIRECTORIES( ${PROJ_BINARY_DIR}/${COMPONENT_NAME} )
  ENDIF(LOCAL_LIBRARY)

  MESSAGE( "Planning to Build Component: ${COMPONENT_NAME}" )

ENDMACRO( GLOBAL_ADD_COMPONENT COMPONENT_NAME )

#
# Components should add tests by calling 'GLOBAL_ADD_TEST' 
# instead of 'ADD_EXECUTABLE' in CMakeLists.txt.
#
# This gives a centralised location where all tests are registered
# and lets us add various things to all components in just one place.
#
#
# Usage: GLOBAL_ADD_TEST( TEST_NAME src1 src2 src3 )
#
MACRO( GLOBAL_ADD_TEST TEST_NAME )
  
  #
  # unless we have a good reason not to, 
  # build and install this component
  #
IF(BUILD_TESTS)
  ADD_EXECUTABLE( ${TEST_NAME} ${ARGN} )
  #INSTALL_TARGETS( /bin ${TEST_NAME} )
  TARGET_LINK_LIBRARIES( ${TEST_NAME} ${OROCOS_RTT_LIBS} )

  MESSAGE( "Planning to build test: ${TEST_NAME}" )
ELSE(BUILD_TESTS)
  MESSAGE( "Not building test: ${TEST_NAME}" )
ENDIF(BUILD_TESTS)


ENDMACRO( GLOBAL_ADD_TEST TEST_NAME )

#
# Components should add library dependencies by calling 'GLOBAL_ADD_DEPENDENCY' 
# This gives a centralised location where all deps are registered
#
# Usage: GLOBAL_ADD_DEPENDENCY( lib1 lib2 ...  )
#
MACRO( GLOBAL_ADD_DEPENDENCY  )
  
    SEPARATE_ARGUMENTS( NARGS )
    SET( COMPONENTS_LIBRARY_DEPS "${COMPONENTS_LIBRARY_DEPS};${NARGS}" )

ENDMACRO( GLOBAL_ADD_DEPENDENCY  )

#
# Macro to check for optional sub-libraries, eg in the case where
# a single component can drive various bits of hardware with the 
# same interface.
#
# It's probably just as easy to write this stuff by hand, but
# using a macro standardizes the trace statements.
#
# USAGE: OPTIONAL_SUB_LIBRARY( 
#                   DESCRIPTION 
#                   SUBDIRECTORY 
#                   OUTPUT_LIBRARY 
#                   LINK_LIBS
#                   OK_TO_BUILD 
#                   DEFINITION_TAG
#                   lib1 lib2 ... libn )
#
# Where:
#  - DESCRIPTION:       Descriptive message
#  - SUBDIRECTORY:      Where the code lives
#  - OUTPUT_LIBRARY:    The optional sub-library
#  - LINK_LIBS:         Extra libraries one needs to link against
#  - OK_TO_BUILD:       The name of the variable that holds whether this sub-library be build
#  - DEFINITION_TAG:    String tag used for both: the compiler flag -Dxxx and the CMake variable.
#                       Both indicate the library can be built.
#  - lib1 ... libn:     The extra libraries that need to be linked against the component
#
MACRO( OPTIONAL_SUB_LIBRARY DESCRIPTION SUBDIRECTORY OUTPUT_LIBRARY LINK_LIBS OK_TO_BUILD DEFINITION_TAG )

  IF( ${${OK_TO_BUILD}} )
    MESSAGE("    ${DESCRIPTION} - can be built")
    SUBDIRS( ${SUBDIRECTORY} )
    
    # The top level executable will be link to this optional libraries...
    SET( SUB_LINK_LIBRARIES ${OUTPUT_LIBRARY} )
    # ... and all the libraries it depends on.
    FOREACH( ARG ${ARGN} )
        SET( SUB_LINK_LIBRARIES ${SUB_LINK_LIBRARIES} ${ARG} )
    ENDFOREACH( ARG ${ARGN} )
    SET( ${LINK_LIBS}  ${SUB_LINK_LIBRARIES} )
    
    LINK_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR}/${SUBDIRECTORY} )
    ADD_DEFINITIONS( -D${DEFINITION_TAG} )
    SET(${DEFINITION_TAG} TRUE)
#    ADD_LIBRARY( ${OUTPUT_LIBRARY} )
  ELSE(  ${${OK_TO_BUILD}} )
    MESSAGE("    ${DESCRIPTION} - cannot be built")
    SET(${DEFINITION_TAG} FALSE)
  ENDIF( ${${OK_TO_BUILD}} )

ENDMACRO( OPTIONAL_SUB_LIBRARY DESCRIPTION DIRECTORY LIBNAME )

