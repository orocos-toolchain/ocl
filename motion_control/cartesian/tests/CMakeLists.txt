IF ( BUILD_TESTS)
OPTION( BUILD_CARTESIAN_TEST "Build Cartesian Components Test" OFF )

IF ( BUILD_CARTESIAN_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( cartesian ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( cartesian orocos-taskbrowser orocos-reporting orocos-viewer orocos-cartesian orocos-naxes orocos-kuka orocos-axes-drivers ${KDL_LIBS} ${KDLTK_LIBS} ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_CARTESIAN_TEST )
ENDIF ( BUILD_TESTS)
