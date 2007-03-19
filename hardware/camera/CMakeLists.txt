DEPENDENT_OPTION( BUILD_CAMERA "Build Camera Components" OFF "BUILD_HARDWARE;OpenCV_FOUND" OFF)

#Always process subdirs
ADD_SUBDIRECTORY( tests )

IF ( BUILD_CAMERA )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-camera ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})
    TARGET_LINK_LIBRARIES(orocos-camera ${OpenCV_LIBRARIES}
      ${GTHREAD_LIBS} ${GTK_LIBS} 
      jpeg
      raw1394
      dc1394_control
      avcodec
      avformat)

ENDIF ( BUILD_CAMERA )
