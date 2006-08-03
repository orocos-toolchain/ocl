IF ( BUILD_TESTS)
OPTION( BUILD_KRYPTON_TEST "Build Krypton Components Test" OFF )

IF ( BUILD_KRYPTON_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( krypton ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( krypton orocos-taskbrowser orocos-krypton orocos-${OROCOS_TARGET} kdltk kdl)


ENDIF ( BUILD_KRYPTON_TEST )
ENDIF ( BUILD_TESTS)