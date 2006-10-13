OPTION( BUILD_IO "Build IO Component" OFF )

IF ( BUILD_IO )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-io ${SRCS} )
    GLOBAL_ADD_INCLUDE( hardware/io ${HPPS} )

    #ADD_SUBDIRECTORY( tests )
    ADD_SUBDIRECTORY( drivers )

ENDIF ( BUILD_IO )
