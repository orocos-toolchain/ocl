DEPENDENT_OPTION( BUILD_XY_ETHERCAT "Build XY Table over EtherCat Component" OFF "BUILD_HARDWARE" OFF)

#Always process subdirs
#ADD_SUBDIRECTORY( tests )

IF ( BUILD_XY_ETHERCAT )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-xy-ethercat ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

    INCLUDE_DIRECTORIES(${PROJ_SOURCE_DIR}/hardware/axes)
    TARGET_LINK_LIBRARIES( orocos-xy-ethercat orocos-axes)

    #TARGET_LINK_LIBRARIES(orocos-xy-ethercat orocos-ethercat)

ENDIF ( BUILD_XY_ETHERCAT )
