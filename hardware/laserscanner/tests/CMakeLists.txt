DEPENDENT_OPTION( BUILD_LASERSCANNER_TEST "Build Laser Distance Sensor Components Test" OFF "BUILD_LASERSCANNER;BUILD_TESTS" OFF)

IF ( BUILD_TESTS)
IF ( BUILD_LASERSCANNER_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( laserscanner ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    PROGRAM_ADD_DEPS( laserscanner orocos-taskbrowser orocos-reporting orocos-laserscanner )

ENDIF ( BUILD_LASERSCANNER_TEST )
ENDIF ( BUILD_TESTS)