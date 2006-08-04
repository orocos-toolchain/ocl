OPTION( BUILD_KUKA "Build Kuka Components" OFF )

IF ( BUILD_KUKA )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )
      
    GLOBAL_ADD_COMPONENT( orocos-kuka ${SRCS} )
    GLOBAL_ADD_INCLUDE( hardware/kuka ${HPPS})
    ADD_SUBDIRECTORY( tests )
    ADD_SUBDIRECTORY( cpf )

ENDIF ( BUILD_KUKA )
