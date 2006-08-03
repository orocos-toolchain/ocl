IF ( BUILD_TESTS)
OPTION( BUILD_LASER_TEST "Build Laser Distance Sensor Components Test" OFF )

IF ( BUILD_LASER_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( laser ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( laser orocos-taskbrowser orocos-reporting orocos-laser orocos-${OROCOS_TARGET})


ENDIF ( BUILD_LASER_TEST )
ENDIF ( BUILD_TESTS)