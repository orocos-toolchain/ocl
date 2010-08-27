OPTION( BUILD_TIMER "Build Timer Component" ON )

IF ( BUILD_TIMER )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB HPPS [^.]*.hpp )
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-timer ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})
     
    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_TIMER )
