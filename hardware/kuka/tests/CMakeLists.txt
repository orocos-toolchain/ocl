DEPENDENT_OPTION( BUILD_KUKA_TEST "Build Kuka Components Test" OFF "BUILD_KUKA;BUILD_TESTS" OFF)

IF ( BUILD_KUKA_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    # FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( kuka main.cpp )
    GLOBAL_ADD_TEST( torquekuka torquemain.cpp )
    GLOBAL_ADD_TEST( encoderTest encoderKuka361Test.cpp )

    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/axes ${PROJECT_SOURCE_DIR}/hardware/comedi ${PROJECT_SOURCE_DIR}/hardware/apci)
    LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/hardware/kuka )
    TARGET_LINK_LIBRARIES( kuka orocos-taskbrowser orocos-reporting orocos-kuka)
    TARGET_LINK_LIBRARIES( torquekuka orocos-taskbrowser orocos-reporting orocos-kuka)

    
    TARGET_LINK_LIBRARIES( kuka ${OROCOS_RTT_LIBS})
    TARGET_LINK_LIBRARIES( encoderTest ${OROCOS_RTT_LIBS} )
    TARGET_LINK_LIBRARIES( torquekuka ${OROCOS_RTT_LIBS} )

ENDIF ( BUILD_KUKA_TEST )
