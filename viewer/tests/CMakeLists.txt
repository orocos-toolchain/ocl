OPTION( BUILD_VIEWER_TEST "Build Viewer Test" ON )

IF ( BUILD_VIEWER_TEST )

    SET (SRCS main.cpp)

    GLOBAL_ADD_TEST( viewer ${SRCS} )

    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( viewer orocos-taskbrowser orocos-reporting orocos-kuka orocos-gnulinux orocos-viewer)

ENDIF ( BUILD_VIEWER_TEST )
