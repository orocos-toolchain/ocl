DEPENDENT_OPTION( BUILD_WRENCH_TEST "Build WrenchSensor Components Test" OFF "BUILD_WRENCH;BUILD_TESTS" OFF)

IF ( BUILD_TESTS)
IF ( BUILD_WRENCH_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( wrench ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( wrench orocos-kdl )
    PROGRAM_ADD_DEPS( wrench orocos-taskbrowser orocos-reporting orocos-wrench orocos-kdltk  )

ENDIF ( BUILD_WRENCH_TEST )
ENDIF ( BUILD_TESTS)
