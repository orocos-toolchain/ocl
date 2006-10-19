IF ( BUILD_TESTS)
OPTION( BUILD_VIEWER_TEST "Build Viewer Test" ON )

IF ( BUILD_VIEWER_TEST )

    SET (SRCS main.cpp)

    GLOBAL_ADD_TEST( viewer ${SRCS} )

    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR}/hardware/axes )
    TARGET_LINK_LIBRARIES( viewer orocos-taskbrowser orocos-reporting orocos-kuka orocos-axes orocos-viewer ACE ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_VIEWER_TEST )
ENDIF ( BUILD_TESTS)
