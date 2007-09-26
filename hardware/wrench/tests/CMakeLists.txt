DEPENDENT_OPTION( BUILD_WRENCH_TEST "Build WrenchSensor Components Test" OFF "BUILD_WRENCH;BUILD_TESTS" OFF)

IF ( BUILD_TESTS)
IF ( BUILD_WRENCH_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( wrench ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( wrench orocos-taskbrowser orocos-reporting orocos-wrench ${OROCOS_RTT_LIBS} orocos-kdltk orocos-kdl ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_WRENCH_TEST )
ENDIF ( BUILD_TESTS)
