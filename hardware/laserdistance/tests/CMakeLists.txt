DEPENDENT_OPTION( BUILD_LASERDISTANCE_TEST "Build Laser Distance Sensor Components Test" OFF "BUILD_LASERDISTANCE;BUILD_TESTS" OFF)

IF ( BUILD_TESTS)
IF ( BUILD_LASERDISTANCE_TEST )

    INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR}/hardware/comedi)

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( laserdistance ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    PROGRAM_ADD_DEPS( laserdistance orocos-taskbrowser orocos-reporting orocos-laserdistance orocos-comedi )
    TARGET_LINK_LIBRARIES( laserdistance ${COMEDI_LIBS} ${OROCOS_RTT_LIBS} )

ENDIF ( BUILD_LASERDISTANCE_TEST )
ENDIF ( BUILD_TESTS)