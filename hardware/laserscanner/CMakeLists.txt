DEPENDENT_OPTION( BUILD_LASERSCANNER "Build Laser Scanner Sensor Components" OFF "BUILD_HARDWARE" OFF)

#Always process subdirs
ADD_SUBDIRECTORY( tests )

IF ( BUILD_LASERSCANNER )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp [^.]*.h )

    GLOBAL_ADD_COMPONENT( orocos-laserscanner ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

ENDIF ( BUILD_LASERSCANNER )
