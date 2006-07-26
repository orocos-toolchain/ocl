OPTION( BUILD_NAXES "Build NAxes Control Components" ON )

IF ( BUILD_NAXES )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-naxes ${SRCS} )

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_NAXES )
