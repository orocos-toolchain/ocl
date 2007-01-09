DEPENDENT_OPTION( BUILD_LASERSCANNER_TEST "Build Laser Distance Sensor Components Test" OFF "BUILD_LASERSCANNER;BUILD_TESTS" OFF)

IF ( BUILD_TESTS)
IF ( BUILD_LASERSCANNER_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( laser ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( laserscanner ${COMEDI_LIBS} orocos-taskbrowser orocos-reporting ${OROCOS_RTT_LIBS} ${COMEDI_LIBS} orocos-comedi orocos-laserscanner ${OROCOS_RTT_LIBS} ${COMEDI_LIBS} orocos-comedi)

ENDIF ( BUILD_LASERSCANNER_TEST )
ENDIF ( BUILD_TESTS)