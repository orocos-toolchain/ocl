OPTION( BUILD_CANPIE "Build Canpie Component" OFF )

IF ( BUILD_CANPIE )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB DEVSRCS dev/[^.]*.cpp )
    FILE( GLOB DEVHPPS dev/[^.]*.hpp )

    IF ( OROCOS_TARGET STREQUAL lxrt )
       SET( SRCS ${SRCS} driver/canpie_lxrtlib.c)
    ENDIF ( OROCOS_TARGET STREQUAL lxrt )

    GLOBAL_ADD_COMPONENT( orocos-canpie-dev ${DEVSRCS} )
    GLOBAL_ADD_INCLUDE( ocr/dev ${DEVHPPS} )

    #ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_CANPIE )
