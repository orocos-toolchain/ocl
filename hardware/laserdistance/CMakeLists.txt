DEPENDENT_OPTION( BUILD_LASER "Build Laser Distance Sensor Components" OFF "BUILD_HARDWARE;BUILD_COMEDI" OFF)

#Always process subdirs
ADD_SUBDIRECTORY( tests )

IF ( BUILD_LASER )

    INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR}/hardware/comedi)

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-laser ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

ENDIF ( BUILD_LASER )
