DEPENDENT_OPTION( BUILD_CAMERA_TEST "Build CameraComponents Test" OFF "BUILD_CAMERA;BUILD_TESTS" OFF)

IF ( BUILD_TESTS)

IF ( BUILD_CAMERA_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( camera ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( camera orocos-taskbrowser orocos-camera ${OROCOS_RTT_LIBS} ${OPENCV_LIBRARIES} libgthread-2.0.so.0 
        libgtk-x11-2.0.so.0 
        libgdk-x11-2.0.so.0 
        libatk-1.0.so.0 
        libpangocairo-1.0.so.0 
        libpangoft2-1.0.so.0 
        libXext.so.6 
        libXinerama.so.1
        libXi.so.6 
        libXrandr.so.2 
        libXcursor.so.1
        libXfixes.so.3 
        libpango-1.0.so.0 
        libcairo.so.2 
        libXrender.so.1
        libfontconfig.so.1
        libfreetype.so.6
        libX11.so.6 
        libgdk_pixbuf-2.0.so.0 
        libgobject-2.0.so.0
        libgmodule-2.0.so.0
        libglib-2.0.so.0 
        libpng12.so.0 
        libtiff.so.4
        libjpeg.so.62
        libz.so.1 
        libraw1394.so.5
        libdc1394_control.so.13 
        libdl.so.2 
        libpthread.so.0
        libstdc++.so.6 
        libm.so.6 
        libgcc_s.so.1
        libc.so.6 
        libXau.so.6 
        libexpat.so.1 
)


ENDIF ( BUILD_CAMERA_TEST )
ENDIF ( BUILD_TESTS)