OPTION( BUILD_WRENCH "Build WrenchSensor Components" ON )

IF ( BUILD_WRENCH )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-wrench ${SRCS} )

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_WRENCH )
