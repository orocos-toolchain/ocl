#
# Include and link against required stuff
#

INCLUDE (${PROJ_SOURCE_DIR}/config/FindOrocosRTT.cmake)

ADD_DEFINITIONS( "-Wall" )

#
# Components should add themselves by calling 'GLOBAL_ADD_EXECUTABLE' 
# instead of 'ADD_EXECUTABLE' in CMakeLists.txt.
#
# This gives a centralised location where all components are registered
# and lets us add various things to all components in just one place.
#
#
# Usage: GLOBAL_ADD_EXECUTABLE( COMPONENT_NAME src1 src2 src3 )
#
MACRO( GLOBAL_ADD_EXECUTABLE COMPONENT_NAME )
  
  #
  # unless we have a good reason not to, 
  # build and install this component
  #
  ADD_EXECUTABLE( ${COMPONENT_NAME} ${ARGN} )
  INSTALL_TARGETS( /bin ${COMPONENT_NAME} )

  MESSAGE( "Planning to Build Component: ${COMPONENT_NAME}" )

ENDMACRO( GLOBAL_ADD_EXECUTABLE COMPONENT_NAME )

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

#
# Project-specific rules
#
INCLUDE( ${PROJ_SOURCE_DIR}/config/project/project_rules.cmake )


#
# Rule for generating .cfg files from .def files
#
MACRO( GENERATE_FROM_DEF DEF_FILE )

  FIND_GENERATE_CFG( GENERATE_CFG_EXE )
  STRING( REGEX REPLACE "\\.def" ".cfg" CFG_FILE ${DEF_FILE} )
  ADD_CUSTOM_TARGET( 
    ${CFG_FILE} ALL
    ${GENERATE_CFG_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/${DEF_FILE} ${CFG_FILE}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DEF_FILE} )
  ADD_DEPENDENCIES( ${CFG_FILE} generatecfg )
  INSTALL_FILES( /cfg FILES ${CFG_FILE} )

ENDMACRO( GENERATE_FROM_DEF DEF_FILE )

#
# Install component definition files (.def files)
#
INSTALL_FILES( /cfg .+\\.cfg$ )
INSTALL_FILES( /def .+\\.def$ )
