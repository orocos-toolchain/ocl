IF ( BUILD_TESTS)
OPTION( BUILD_WRENCH_TEST "Build WrenchSensor Components Test" OFF )

IF ( BUILD_WRENCH_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( wrench ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( wrench orocos-taskbrowser orocos-reporting orocos-wrench ${OROCOS_RTT_LIBS} kdltk kdl ${OROCOS_RTT_LIBS})

    ADD_SUBDIRECTORY( cpf )

ENDIF ( BUILD_WRENCH_TEST )
ENDIF ( BUILD_TESTS)
