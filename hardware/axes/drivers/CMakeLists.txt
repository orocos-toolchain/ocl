
    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-axes-drivers ${SRCS} )
    GLOBAL_ADD_INCLUDE( hardware/axes ${HPPS} )

