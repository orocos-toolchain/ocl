OPTION( BUILD_REPORTING "Build Reporting Component" ON )

IF ( BUILD_REPORTING )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp tcp/[^.]*.cpp)
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-reporting ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})
    
    ADD_SUBDIRECTORY( tests )
ENDIF ( BUILD_REPORTING )
