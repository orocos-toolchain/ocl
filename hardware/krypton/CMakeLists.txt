OPTION( BUILD_KRYPTON "Build Krypton Components" OFF )

IF ( BUILD_KRYPTON )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-krypton ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_KRYPTON )


