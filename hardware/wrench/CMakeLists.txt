OPTION( BUILD_WRENCH "Build WrenchSensor Components" OFF )

IF ( BUILD_WRENCH )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-wrench ${SRCS} )
    GLOBAL_ADD_INCLUDE( hardware/wrench ${HPPS} )
    GLOBAL_ADD_INCLUDE( hardware/wrench/driver/ driver/jr3_lxrt_common.h )

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_WRENCH )
