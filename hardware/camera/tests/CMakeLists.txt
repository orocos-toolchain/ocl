IF ( BUILD_TESTS)
OPTION( BUILD_CAMERA_TEST "Build CameraComponents Test" OFF )

IF ( BUILD_CAMERA_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( camera ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( camera orocos-taskbrowser orocos-camera)


ENDIF ( BUILD_CAMERA_TEST )
ENDIF ( BUILD_TESTS)