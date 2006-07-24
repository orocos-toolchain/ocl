OPTION( BUILD_REPORTING_TEST "Build Reporting Test" ON )

IF ( BUILD_REPORTING_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_TEST( report ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( report ncurses readline orocos-taskbrowser orocos-reporting)

ENDIF ( BUILD_REPORTING_TEST )
