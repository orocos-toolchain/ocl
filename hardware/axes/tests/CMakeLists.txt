DEPENDENT_OPTION( BUILD_AXES_TEST "Build Axes Test" OFF BUILD_TESTS OFF)

IF ( BUILD_AXES_TEST )

    FILE( GLOB MSRCS main.cpp )
    FILE( GLOB MCSRCS main_channels.cpp )

    GLOBAL_ADD_TEST( axes ${MSRCS} )
    GLOBAL_ADD_TEST( axes_channels ${MCSRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( axes orocos-taskbrowser orocos-axes ${OROCOS_RTT_LIBS})
    TARGET_LINK_LIBRARIES( axes_channels orocos-taskbrowser orocos-axes ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_AXES_TEST )
