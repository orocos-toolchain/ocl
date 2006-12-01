DEPENDENT_OPTION( BUILD_KRYPTON "Build Krypton Components" OFF "BUILD_HARDWARE" OFF)

#Always process subdirs
ADD_SUBDIRECTORY( tests )

IF ( BUILD_KRYPTON )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-krypton ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

ENDIF ( BUILD_KRYPTON )


