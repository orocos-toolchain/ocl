#
# This is a global registry of all components.

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