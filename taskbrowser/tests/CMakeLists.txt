
CMAKE_DEPENDENT_OPTION( BUILD_TASKBROWSER_TEST "Build TaskBrowser Test" ON  "BUILD_TASKBROWSER;BUILD_TESTS" OFF)

IF ( BUILD_TASKBROWSER_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( taskb ${SRCS} )
    program_add_deps( taskb ${OROCOS-RTT_LIBRARIES} orocos-ocl-taskbrowser )

ENDIF ( BUILD_TASKBROWSER_TEST )

