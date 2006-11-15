DEPENDENT_OPTION( BUILD_CAMERA "Build Camera Components" OFF "BUILD_HARDWARE" OFF)

IF ( BUILD_CAMERA )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-camera ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_CAMERA )
