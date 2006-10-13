OPTION( BUILD_AXES "Build Axes Component" OFF )

IF ( BUILD_AXES )

    # This gathers all the .cpp files into the variable 'SRCS'
    #FILE( GLOB SRCS [^.]*.cpp )
    #FILE( GLOB HPPS [^.]*.hpp )

    #GLOBAL_ADD_COMPONENT( orocos-axes ${SRCS} )
    #GLOBAL_ADD_INCLUDE( hardware/axes ${HPPS} )

    #ADD_SUBDIRECTORY( tests )
    ADD_SUBDIRECTORY( drivers )

ENDIF ( BUILD_AXES )
