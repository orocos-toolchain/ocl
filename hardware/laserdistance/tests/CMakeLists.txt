DEPENDENT_OPTION( BUILD_LASER_TEST "Build Laser Distance Sensor Components Test" OFF "BUILD_LASER;BUILD_TESTS" OFF)

IF ( BUILD_TESTS)
IF ( BUILD_LASER_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( laser ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( laser ${COMEDI_LIBS} orocos-taskbrowser orocos-reporting ${OROCOS_RTT_LIBS} ${COMEDI_LIBS} orocos-comedi orocos-laser ${OROCOS_RTT_LIBS} ${COMEDI_LIBS} orocos-comedi)

ENDIF ( BUILD_LASER_TEST )
ENDIF ( BUILD_TESTS)