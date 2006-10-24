DEPENDENT_OPTION( BUILD_AXES_TEST "Build Axes Test" OFF BUILD_TESTS OFF)

IF ( BUILD_AXES_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_TEST( axes ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( axes orocos-taskbrowser orocos-axes ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_AXES_TEST )
