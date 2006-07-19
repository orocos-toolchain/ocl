# Locate Ice home

# This module defines
# ICE_HOME where to find include, lib, bin, etc.
# ICE_FOUND, If false, don't try to use Ice.


FIND_PATH( ICE_INCLUDE_DIR Ice.h
  $ENV{ICE_HOME}/include/Ice
  /usr/local/include/Ice
  /opt/Ice-2.1.2/include
  /opt/Ice-3.0.0/include
  /opt/Ice-3.0.1/include
  #C:/Progra~1/Ice-2.1.2/include
  )

IF ( "${ICE_INCLUDE_DIR}" STREQUAL ${ICE_INCLUDE_DIR}-NOTFOUND )
    SET( ICE_FOUND NO
        CACHE BOOL "Do we have Ice?" )
    MESSAGE ( "   FindIce: Ice not found" )
ELSE ( "${ICE_INCLUDE_DIR}" STREQUAL ${ICE_INCLUDE_DIR}-NOTFOUND )
    SET( ICE_FOUND YES
        CACHE BOOL "Do we have Ice?" )
ENDIF ( "${ICE_INCLUDE_DIR}" STREQUAL ${ICE_INCLUDE_DIR}-NOTFOUND )

# this is ugly but don't know how to get the parent of a directory

IF ( "${ICE_INCLUDE_DIR}" STREQUAL "$ENV{ICE_HOME}/include/Ice" )
    SET ( ICE_HOME $ENV{ICE_HOME} )
    MESSAGE ( "   FindIce:  found in ${ICE_HOME}" )
ENDIF ( "${ICE_INCLUDE_DIR}" STREQUAL "$ENV{ICE_HOME}/include/Ice" )

IF ( "${ICE_INCLUDE_DIR}" STREQUAL "/usr/local/include/Ice" )
    SET ( ICE_HOME /usr/local )
    MESSAGE ( "   FindIce:  found in ${ICE_HOME}" )
ENDIF ( "${ICE_INCLUDE_DIR}" STREQUAL "/usr/local/include/Ice" )

IF ( "${ICE_INCLUDE_DIR}" STREQUAL /opt/Ice-2.1.2/include/Ice )
    SET ( ICE_HOME /opt/Ice-2.1.2 )
    MESSAGE ( "   FindIce:  found in ${ICE_HOME}" )
ENDIF ( "${ICE_INCLUDE_DIR}" STREQUAL /opt/Ice-2.1.2/include/Ice )

IF ( "${ICE_INCLUDE_DIR}" STREQUAL /opt/Ice-3.0.0/include/Ice )
    SET ( ICE_HOME /opt/Ice-3.0.0 )
   MESSAGE ( "   FindIce:  found in ${ICE_HOME}" )
ENDIF ( "${ICE_INCLUDE_DIR}" STREQUAL /opt/Ice-3.0.0/include/Ice )

IF ( "${ICE_INCLUDE_DIR}" STREQUAL /opt/Ice-3.0.1/include/Ice )
    SET ( ICE_HOME /opt/Ice-3.0.1 )
   MESSAGE ( "   FindIce:  found in ${ICE_HOME}" )
ENDIF ( "${ICE_INCLUDE_DIR}" STREQUAL /opt/Ice-3.0.1/include/Ice )

SET( ICE_HOME ${ICE_HOME} CACHE PATH "Ice installed directory" FORCE )
  MESSAGE( "Setting Ice installed directory to ${ICE_HOME}" )

#FIND_LIBRARY(ICE_ICE_LIBRARY
#  NAMES Ice
#  PATHS
#  $ENV{ICE_HOME}/lib/Ice
#  /usr/local/lib/Ice
#  /opt/Ice-2.1.2/lib
#  /opt/Ice-3.0.0/lib
#  C:/Progra~1/Ice-2.1.2/Include
#  )