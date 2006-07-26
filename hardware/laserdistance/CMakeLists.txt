OPTION( BUILD_LASER "Build Laser Distance Sensor Components" OFF )

IF ( BUILD_LASER )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-laser ${SRCS} )

    ADD_SUBDIRECTORY( tests )
    ADD_SUBDIRECTORY( cpf )

ENDIF ( BUILD_LASER )
