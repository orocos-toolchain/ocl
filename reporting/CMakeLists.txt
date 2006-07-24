OPTION( BUILD_REPORTING "Build Reporting Component" ON )

IF ( BUILD_REPORTING )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-reporting ${SRCS} )

    ADD_SUBDIRECTORY( tests )
ENDIF ( BUILD_REPORTING )
