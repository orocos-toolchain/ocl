
DEPENDENT_OPTION( BUILD_KUKA_TEST "Build Kuka Components Test" OFF "BUILD_KUKA;BUILD_TESTS" OFF)

IF ( BUILD_KUKA_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    # FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( kuka main.cpp )
    GLOBAL_ADD_TEST( torquekuka torquemain.cpp )
    GLOBAL_ADD_TEST( encoderTest encoderKuka361Test.cpp )

    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/axes )
    TARGET_LINK_LIBRARIES( kuka orocos-taskbrowser orocos-reporting orocos-kuka orocos-axes)
    TARGET_LINK_LIBRARIES( torquekuka orocos-taskbrowser orocos-reporting orocos-kuka orocos-axes)

    IF ( BUILD_COMEDI )
    TARGET_LINK_LIBRARIES( kuka orocos-comedi )
    TARGET_LINK_LIBRARIES( torquekuka orocos-comedi )
    ENDIF ( BUILD_COMEDI )

    IF ( BUILD_APCI )
    MESSAGE( "BUILD_APCI : ${BUILD_APCI}" )
    TARGET_LINK_LIBRARIES( kuka orocos-apci )
    TARGET_LINK_LIBRARIES( torquekuka orocos-apci )
    TARGET_LINK_LIBRARIES( encoderTest orocos-apci )
    ENDIF ( BUILD_APCI )
    
    TARGET_LINK_LIBRARIES( kuka ${OROCOS_RTT_LIBS})
    TARGET_LINK_LIBRARIES( torquekuka ${OROCOS_RTT_LIBS})
    TARGET_LINK_LIBRARIES( encoderTest ${OROCOS_RTT_LIBS} )

ENDIF ( BUILD_KUKA_TEST )
