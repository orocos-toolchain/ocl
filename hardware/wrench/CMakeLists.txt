OPTION( BUILD_WRENCH "Build WrenchSensor Components" ON )

IF ( BUILD_WRENCH )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-wrench ${SRCS} )
    GLOBAL_ADD_INCLUDE( hardware/wrench ${HPPS})

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_WRENCH )
