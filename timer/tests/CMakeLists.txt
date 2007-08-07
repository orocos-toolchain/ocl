
DEPENDENT_OPTION( BUILD_TIMER_TEST "Build Timer Test" ON "BUILD_TIMER;BUILD_TESTS" OFF)

IF ( BUILD_TIMER_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'

    GLOBAL_ADD_TEST( timer main.cpp )
    # Use  TARGET_LINK_LIBRARIES( timer libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( timer orocos-taskbrowser orocos-timer  ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_TIMER_TEST )

