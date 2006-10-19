OPTION( BUILD_BECKHOFF "Build Beckhoff Component" OFF )

IF ( BUILD_BECKHOFF )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB DEVSRCS dev/[^.]*.cpp )
    FILE( GLOB DEVHPPS dev/[^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-beckhoff-dev ${DEVSRCS} )
    GLOBAL_ADD_INCLUDE( ocr/dev ${DEVHPPS} )

    TARGET_USE_LIB( orocos-beckhoff-dev hardware/canpie orocos-canpie-dev)

    #ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_BECKHOFF )
