IF ( BUILD_TESTS)
OPTION( BUILD_KUKA_TEST "Build Kuka Components Test" OFF )

IF ( BUILD_KUKA_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    # FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( kuka main.cpp )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( kuka orocos-taskbrowser orocos-reporting orocos-kuka orocos-axes-drivers ${OROCOS_RTT_LIBS})
    
    GLOBAL_ADD_TEST( encoderTest encoderKuka361Test.cpp )
    TARGET_LINK_LIBRARIES( encoderTest ${OROCOS_RTT_LIBS})

    IF ( OROCOS_TARGET STREQUAL lxrt )
      TARGET_LINK_LIBRARIES( kuka orocos-apci )
      TARGET_LINK_LIBRARIES( encoderTest orocos-apci )
    ENDIF ( OROCOS_TARGET STREQUAL lxrt )

ENDIF ( BUILD_KUKA_TEST )
ENDIF ( BUILD_TESTS)
