IF ( BUILD_TESTS)
OPTION( BUILD_NAXES_TEST "Build NAxes Components Test" OFF )

IF ( BUILD_NAXES_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( naxes ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( naxes orocos-taskbrowser orocos-reporting orocos-naxes orocos-kuka orocos-axes-drivers orocos-viewer ${OROCOS_RTT_LIBS} kdl ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_NAXES_TEST )
ENDIF ( BUILD_TESTS)
