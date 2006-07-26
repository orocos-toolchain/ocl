OPTION( BUILD_KUKA "Build Kuka Components" OFF )

IF ( BUILD_KUKA )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-kuka ${SRCS} )
    ADD_SUBDIRECTORY( tests )
    ADD_SUBDIRECTORY( cpf )

ENDIF ( BUILD_KUKA )
