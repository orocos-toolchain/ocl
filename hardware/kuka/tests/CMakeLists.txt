IF ( BUILD_TESTS)
OPTION( BUILD_KUKA_TEST "Build Kuka Components Test" OFF )

IF ( BUILD_KUKA_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( kuka ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( kuka orocos-taskbrowser orocos-reporting orocos-kuka ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_KUKA_TEST )
ENDIF ( BUILD_TESTS)
