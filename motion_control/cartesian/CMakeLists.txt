OPTION( BUILD_CARTESIAN "Build Cartesian Control Components" ON )

IF ( BUILD_CARTESIAN )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-cartesian ${SRCS} )
    GLOBAL_ADD_INCLUDE( motion_control/cartesian ${HPPS})

    ADD_SUBDIRECTORY( tests )
    ADD_SUBDIRECTORY( cpf )

ENDIF ( BUILD_CARTESIAN )
