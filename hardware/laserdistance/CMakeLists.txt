OPTION( BUILD_LASER "Build Laser Distance Sensor Components" OFF )

IF ( BUILD_LASER )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-laser ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_LASER )
