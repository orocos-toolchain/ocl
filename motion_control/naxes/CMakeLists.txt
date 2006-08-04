OPTION( BUILD_NAXES "Build NAxes Control Components" ON )

IF ( BUILD_NAXES )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-naxes ${SRCS} )
    GLOBAL_ADD_INCLUDE( motion_control/naxes ${HPPS})

    ADD_SUBDIRECTORY( tests )
    ADD_SUBDIRECTORY( cpf )

ENDIF ( BUILD_NAXES )
