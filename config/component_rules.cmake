#
# Include and link against required stuff
#

ADD_DEFINITIONS( "-Wall" )


#
# Components supply header files which should be included when 
# using these components. Each component should use this macro
# to supply its header-files.
#
# Usage: GLOBAL_ADD_INCLUDE( RELATIVE_LOCATION hpp1, hpp2 ...)
MACRO( GLOBAL_ADD_INCLUDE COMPONENT_LOCATION )
  INSTALL_FILES( /include/${COMPONENT_LOCATION} FILES ${ARGN})
ENDMACRO( GLOBAL_ADD_INCLUDE COMPONENT_LOCATION )

# Link a component library with an external library (qt3, gl, readline,....)
# Usage: COMPONENT_ADD_LIBS( orocos-taskbrowser readline ncurses )
MACRO( COMPONENT_ADD_LIBS COMPONENT_NAME  )
  foreach( lib ${ARGN} )
    TARGET_LINK_LIBRARIES( ${COMPONENT_NAME}-${OROCOS_TARGET} ${lib} )
  endforeach( lib ${ARGN} )
ENDMACRO( COMPONENT_ADD_LIBS COMPONENT_NAME )

# Link a component library with another component library
# Usage: COMPONENT_ADD_DEPS( orocos-taskbrowser orocos-reporting )
MACRO( COMPONENT_ADD_DEPS COMPONENT_NAME )
  foreach( lib ${ARGN} )
    TARGET_LINK_LIBRARIES( ${COMPONENT_NAME}-${OROCOS_TARGET} ${lib}-${OROCOS_TARGET} )
  endforeach( lib ${ARGN} )
ENDMACRO( COMPONENT_ADD_DEPS COMPONENT_NAME )

# Link a program with a component library
# Usage: PROGRAM_ADD_DEPS( taskbrowser-test orocos-reporting )
MACRO( PROGRAM_ADD_DEPS PROGRAM_NAME )
  foreach( lib ${ARGN} )
    TARGET_LINK_LIBRARIES( ${PROGRAM_NAME} ${lib}-${OROCOS_TARGET} )
  endforeach( lib ${ARGN} )
ENDMACRO( PROGRAM_ADD_DEPS PROGRAM_NAME )


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
  
#  IF(STANDALONE_COMPONENTS)
#     MESSAGE( "BROKEN: Building Stand-alone component ${COMPONENT_NAME}" )
#     ADD_EXECUTABLE( ${COMPONENT_NAME} ${ARGN} )
#     #INSTALL_TARGETS( /bin ${COMPONENT_NAME} )
#     TARGET_LINK_LIBRARIES( ${COMPONENT_NAME} ${OROCOS_RTT_LIBS} )
#  ENDIF(STANDALONE_COMPONENTS)

  SET( LIB_NAME "${COMPONENT_NAME}-${OROCOS_TARGET}")
  
  IF(GLOBAL_LIBRARY)
     MESSAGE( ERROR "BROKEN: Adding ${COMPONENT_NAME} to global sources:[ ${GLOBAL_LIBRARY_SRCS} ]" )
     SET (GLOBAL_LIBRARY_SRCS "${GLOBAL_LIBRARY_SRCS} ${ARGN}" )
  ENDIF(GLOBAL_LIBRARY)

  IF(LOCAL_LIBRARY)
    IF (OROCOS_RTT_1.4)
      MESSAGE( "Building Shared library for ${COMPONENT_NAME}" )
      ADD_LIBRARY( ${LIB_NAME} SHARED ${ARGN} )
      SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES 
	DEFINE_SYMBOL OCL_DLL_EXPORT 
	VERSION ${OCL_VERSION}
	SOVERSION ${OCL_VERSION_MAJOR}.${OCL_VERSION_MINOR}
	)
      foreach(lib ${OROCOS_RTT_LIBS})
 	TARGET_LINK_LIBRARIES( ${LIB_NAME} ${lib} )
      endforeach(lib in ${OROCOS_RTT_LIBS})

    ELSE (OROCOS_RTT_1.4)
      IF (OROCOS_RTT_1.2)
	MESSAGE( "Building Shared library for ${COMPONENT_NAME}" )
	ADD_LIBRARY( ${LIB_NAME} SHARED ${ARGN} )
	SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES 
	  DEFINE_SYMBOL OCL_DLL_EXPORT 
	  VERSION ${OCL_VERSION}
	  SOVERSION ${OCL_VERSION_MAJOR}.${OCL_VERSION_MINOR}
	  )
	TARGET_LINK_LIBRARIES( ${LIB_NAME} orocos-rtt )
      ELSE (OROCOS_RTT_1.2)
	MESSAGE( "Building Static library for ${COMPONENT_NAME}" )
	ADD_LIBRARY( ${LIB_NAME} STATIC ${ARGN} )
      ENDIF (OROCOS_RTT_1.2)
    ENDIF (OROCOS_RTT_1.4)

    INSTALL_TARGETS( /lib ${LIB_NAME} )
    #The later a component is added, the earlier it apears in the -l list.
    SET (ENV{SELECTED_LIBS} "-l${LIB_NAME} $ENV{SELECTED_LIBS} ")
    #This is an ugly work around
    #FILE(APPEND ${PROJ_BINARY_DIR}/bibs.txt "-l${LIB_NAME} ")

    LINK_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
    SET (ENV{SELECTED_DIRS} "$ENV{SELECTED_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}")
    
  ENDIF(LOCAL_LIBRARY)

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
	#INSTALL_TARGETS( /bin/${TEST_NAME} ${TEST_NAME} )

	MESSAGE("Planning to build test: ${TEST_NAME}" )
  ELSE(BUILD_TESTS)
	MESSAGE("Not building test: ${TEST_NAME}" )
  ENDIF(BUILD_TESTS)
ENDMACRO( GLOBAL_ADD_TEST TEST_NAME )

#
# Components should add library dependencies by calling 'GLOBAL_ADD_DEPENDENCY' 
# This gives a centralised location where all deps are registered
#
# Usage: GLOBAL_ADD_DEPENDENCY( lib1 lib2 ...  )
#
MACRO( GLOBAL_ADD_DEPENDENCY  )
  
    SEPARATE_ARGUMENTS( ARGN )
    SET( COMPONENTS_LIBRARY_DEPS "${COMPONENTS_LIBRARY_DEPS};${ARGN}" )

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
    MESSAGE( "    ${DESCRIPTION} - cannot be built")
    SET(${DEFINITION_TAG} FALSE)
  ENDIF( ${${OK_TO_BUILD}} )

ENDMACRO( OPTIONAL_SUB_LIBRARY DESCRIPTION DIRECTORY LIBNAME )

# Use this to add link flags which get written to the .pc file
MACRO( OROCOS_PKGCONFIG_LIBS TO_ADD)
  foreach( ITEM ${TO_ADD})
    SET( ENV{OROCOS_COMPONENTS_LINKFLAGS} "$ENV{OROCOS_COMPONENTS_LINKFLAGS} -l${ITEM}")
  endforeach( ITEM )
ENDMACRO( OROCOS_PKGCONFIG_LIBS TO_ADD)
   
# Use this to add C flags which get written to the .pc file
MACRO( OROCOS_PKGCONFIG_CFLAGS TO_ADD)
	  SET( ENV{OROCOS_COMPONENTS_CFLAGS} "$ENV{OROCOS_COMPONENTS_CFLAGS} ${TO_ADD}")
ENDMACRO( OROCOS_PKGCONFIG_CFLAGS TO_ADD)

# Use this to add an include path which get written to the .pc file
MACRO( OROCOS_PKGCONFIG_INCPATH TO_ADD)
  foreach( ITEM ${TO_ADD} )
  if ( NOT "${ITEM}" STREQUAL "/usr/include" AND NOT "${ITEM}" STREQUAL "/usr/local/include")
    SET( ENV{OROCOS_COMPONENTS_CFLAGS} "$ENV{OROCOS_COMPONENTS_CFLAGS} -I${ITEM}")
  else ( NOT "${ITEM}" STREQUAL "/usr/include" AND NOT "${ITEM}" STREQUAL "/usr/local/include")
    MESSAGE( "Skipping include path '${ITEM}'")
  endif ( NOT "${ITEM}" STREQUAL "/usr/include" AND NOT "${ITEM}" STREQUAL "/usr/local/include")
  endforeach( ITEM )
ENDMACRO( OROCOS_PKGCONFIG_INCPATH TO_ADD)

# Use this to add a library path which get written to the .pc file
MACRO( OROCOS_PKGCONFIG_LIBPATH TO_ADD)
  foreach( ITEM ${TO_ADD} )
  if ( NOT "${ITEM}" STREQUAL "/usr/lib" AND NOT "${ITEM}" STREQUAL "/usr/local/lib")
    SET( ENV{OROCOS_COMPONENTS_LINKFLAGS} "$ENV{OROCOS_COMPONENTS_LINKFLAGS} -L${ITEM}")
  else ( NOT "${ITEM}" STREQUAL "/usr/lib" AND NOT "${ITEM}" STREQUAL "/usr/local/lib")
    MESSAGE( "Skipping lib path ${ITEM}")
  endif ( NOT "${ITEM}" STREQUAL "/usr/lib" AND NOT "${ITEM}" STREQUAL "/usr/local/lib")
  endforeach( ITEM )
ENDMACRO( OROCOS_PKGCONFIG_LIBPATH TO_ADD)

# Use this to add a .pc dependency ('Requires') which get written to the .pc file
MACRO( OROCOS_PKGCONFIG_REQUIRES TO_ADD)
  SET( ENV{OROCOS_COMPONENTS_REQUIRES} "$ENV{OROCOS_COMPONENTS_REQUIRES} ${TO_ADD}")
ENDMACRO( OROCOS_PKGCONFIG_REQUIRES TO_ADD)

# Use this if you have a dependency on another orocos component
# it sets the include path to SUBDIRECTORY and SUBDIRECTORY/dev
# and adds the correct link flags.
# TARGET_USE_LIB( target hardware/io orocos-io )
MACRO( TARGET_USE_LIB TARGET_NAME SUBDIRECTORY LIBNAME )
    TARGET_LINK_LIBRARIES( ${TARGET_NAME} ${LIBNAME} )
    LINK_DIRECTORIES( ${PROJECT_BINARY_DIR}/${SUBDIRECTORY} )
    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/${SUBDIRECTORY} )
    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/${SUBDIRECTORY}/dev )
ENDMACRO( TARGET_USE_LIB DIRECTORY LIBNAME )