IF ( BUILD_TESTS)

OPTION( BUILD_TASKBROWSER_TEST "Build TaskBrowser Test" OFF )

IF ( BUILD_TASKBROWSER_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( taskb ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( taskb orocos-taskbrowser ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_TASKBROWSER_TEST )

ENDIF ( BUILD_TESTS)