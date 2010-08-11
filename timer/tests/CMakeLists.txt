
DEPENDENT_OPTION( BUILD_TIMER_TEST "Build Timer Test" ON "BUILD_TIMER;BUILD_TESTS" OFF)

IF ( BUILD_TIMER_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'

    GLOBAL_ADD_TEST( timer main.cpp )
    # Use  TARGET_LINK_LIBRARIES( timer libs... ) to add library deps.
    PROGRAM_ADD_DEPS( timer orocos-taskbrowser orocos-timer )

    GLOBAL_ADD_TEST( testWithStateMachine testWithStateMachine.cpp )
    
    find_package(RTTPlugin REQUIRED rtt-marshalling rtt-scripting)
    
    PROGRAM_ADD_DEPS( testWithStateMachine orocos-taskbrowser orocos-timer )
    TARGET_LINK_LIBRARIES( testWithStateMachine ${RTT_PLUGIN_rtt-scripting_LIBRARIES} )
    # Copy over state machine file
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/testWithStateMachine.osd
                   ${CMAKE_CURRENT_BINARY_DIR}/testWithStateMachine.osd COPYONLY)

ENDIF ( BUILD_TIMER_TEST )

