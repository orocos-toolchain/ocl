IF ( BUILD_TESTS)
OPTION( BUILD_DEMOTOOL_TEST "Build Demotool Sensor Components Test" OFF )

IF ( BUILD_DEMOTOOL_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( demotool ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( demotool orocos-wrench orocos-krypton kdltk kdl orocos-taskbrowser orocos-reporting orocos-demotool orocos-gnulinux ${BFL_LIBS})


ENDIF ( BUILD_DEMOTOOL_TEST )
ENDIF ( BUILD_TESTS)