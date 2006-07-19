OPTION( BUILD_TASKBROWSER_TEST "Build TaskBrowser Test" ON )

IF ( BUILD_TASKBROWSER_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS *.cpp )

    LINK_DIRECTORIES( ${PROJ_BINARY_DIR}/taskbrowser )
    INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR}/taskbrowser )

    GLOBAL_ADD_EXECUTABLE( taskb ${SRCS} )
    TARGET_LINK_LIBRARIES( taskb ncurses readline taskbrowser )

ENDIF ( BUILD_TASKBROWSER_TEST )
