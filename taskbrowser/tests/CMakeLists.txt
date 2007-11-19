
DEPENDENT_OPTION( BUILD_TASKBROWSER_TEST "Build TaskBrowser Test" ON  "BUILD_TASKBROWSER;BUILD_TESTS" OFF)

IF ( BUILD_TASKBROWSER_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( taskb ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    PROGRAM_ADD_DEPS( taskb orocos-taskbrowser)

ENDIF ( BUILD_TASKBROWSER_TEST )

