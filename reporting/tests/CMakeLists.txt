
DEPENDENT_OPTION( BUILD_REPORTING_TEST "Build Reporting Test" ON "BUILD_REPORTING;BUILD_TESTS" OFF)

IF ( BUILD_REPORTING_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_TEST( report ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( report orocos-taskbrowser orocos-reporting  ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_REPORTING_TEST )

