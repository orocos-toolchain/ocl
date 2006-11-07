IF ( BUILD_TESTS)
OPTION( BUILD_KUKA_TEST "Build Kuka Components Test" OFF )

IF ( BUILD_KUKA_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    # FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( kuka main.cpp )
    GLOBAL_ADD_TEST( torquekuka torquemain.cpp )
    GLOBAL_ADD_TEST( encoderTest encoderKuka361Test.cpp )

    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/axes )
    TARGET_LINK_LIBRARIES( kuka orocos-taskbrowser orocos-reporting orocos-kuka orocos-axes ${OROCOS_RTT_LIBS})
    TARGET_LINK_LIBRARIES( torquekuka orocos-taskbrowser orocos-reporting orocos-kuka orocos-axes ${OROCOS_RTT_LIBS})
    TARGET_LINK_LIBRARIES( encoderTest ${OROCOS_RTT_LIBS} )

    IF ( BUILD_COMEDI )
    TARGET_LINK_LIBRARIES( kuka orocos-comedi )
    TARGET_LINK_LIBRARIES( torquekuka orocos-comedi )
    ENDIF ( BUILD_COMEDI )

    IF ( BUILD_APCI )
    TARGET_LINK_LIBRARIES( kuka orocos-apci )
    TARGET_LINK_LIBRARIES( torquekuka orocos-apci )
    TARGET_LINK_LIBRARIES( encoderTest orocos-apci )
    ENDIF ( BUILD_APCI )
    

ENDIF ( BUILD_KUKA_TEST )
ENDIF ( BUILD_TESTS)
