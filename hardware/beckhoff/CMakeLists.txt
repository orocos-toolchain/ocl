OPTION( BUILD_BECKHOFF "Build Beckhoff Component" OFF )

IF ( BUILD_BECKHOFF )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-beckhoff ${SRCS} )
    GLOBAL_ADD_INCLUDE( hardware/beckhoff ${HPPS} )
    #GLOBAL_ADD_INCLUDE( hardware/beckhoff/driver/ driver/jr3_lxrt_common.h )

    #ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_BECKHOFF )
