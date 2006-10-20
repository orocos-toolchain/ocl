OPTION( BUILD_BECKHOFF "Build Beckhoff Component" OFF )

IF ( BUILD_BECKHOFF )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB DEVSRCS dev/[^.]*.cpp )
    FILE( GLOB DEVHPPS dev/[^.]*.hpp )

    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/can )
    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/can/can )
    GLOBAL_ADD_COMPONENT( orocos-beckhoff ${DEVSRCS} )
    GLOBAL_ADD_INCLUDE( ocl/dev ${DEVHPPS} )

    #ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_BECKHOFF )
