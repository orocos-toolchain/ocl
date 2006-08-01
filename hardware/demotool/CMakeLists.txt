OPTION( BUILD_DEMOTOOL "Build DEMOTOOL Sensor Components" OFF )

IF ( BUILD_DEMOTOOL )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-demotool ${SRCS} )

    ADD_SUBDIRECTORY( tests )
    ADD_SUBDIRECTORY( cpf )

ENDIF ( BUILD_DEMOTOOL )
