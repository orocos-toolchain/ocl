OPTION( BUILD_DEMOTOOL "Build DEMOTOOL Sensor Components" OFF )

IF ( BUILD_DEMOTOOL )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-demotool ${SRCS} )
    GLOBAL_ADD_INCLUDE( hardware/demotool ${HPPS})

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_DEMOTOOL )
