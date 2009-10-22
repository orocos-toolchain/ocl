DEPENDENT_OPTION( BUILD_KRYPTON_TEST "Build Krypton Components Test" OFF "BUILD_KRYPTON;BUILD_TESTS" OFF)

IF ( BUILD_TESTS)

IF ( BUILD_KRYPTON_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( krypton ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( krypton orocos-kdl )
    PROGRAM_ADD_DEPS( krypton orocos-taskbrowser orocos-krypton orocos-reporting orocos-kdltk )

ENDIF ( BUILD_KRYPTON_TEST )
ENDIF ( BUILD_TESTS)