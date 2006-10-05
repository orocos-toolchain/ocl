OPTION( BUILD_CANPIE "Build Canpie Component" ON )

IF ( BUILD_CANPIE )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-canpie ${SRCS} driver/canpie_lxrtlib.c)
    GLOBAL_ADD_INCLUDE( hardware/canpie ${HPPS} )
    #GLOBAL_ADD_INCLUDE( hardware/canpie/driver/ driver/jr3_lxrt_common.h )

    #ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_CANPIE )
