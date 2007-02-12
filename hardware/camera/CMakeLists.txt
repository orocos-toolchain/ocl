DEPENDENT_OPTION( BUILD_CAMERA "Build Camera Components" OFF "BUILD_HARDWARE;OpenCV_FOUND" OFF)

MESSAGE("OPENCV_FOUND ${OPENCV_FOUND}")

#Always process subdirs
ADD_SUBDIRECTORY( tests )

IF ( BUILD_CAMERA )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-camera ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

ENDIF ( BUILD_CAMERA )
