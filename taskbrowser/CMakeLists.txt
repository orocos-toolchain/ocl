OPTION( BUILD_TASKBROWSER "Build TaskBrowser" ON )

IF ( BUILD_TASKBROWSER )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    GLOBAL_ADD_COMPONENT( orocos-taskbrowser ${SRCS} )
     
    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_TASKBROWSER )
