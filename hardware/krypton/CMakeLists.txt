OPTION( BUILD_KRYPTON "Build Krypton Components" OFF )

IF ( BUILD_KRYPTON )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-krypton ${SRCS} )

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_KRYPTON )
