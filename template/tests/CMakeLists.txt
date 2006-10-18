OPTION( BUILD_TEMPLATE_TEST "Build template test" ON )

IF ( BUILD_TEMPLATE_TEST )

    # Add a test program
    GLOBAL_ADD_TEST( main.cpp )

    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( orocos-taskbrowser orocos-reporting ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_TEMPLATE_TEST )
