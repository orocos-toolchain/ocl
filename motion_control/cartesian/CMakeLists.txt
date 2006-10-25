OPTION( BUILD_CARTESIAN "Build Cartesian Control Components" ON )

IF ( BUILD_CARTESIAN )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/motion_control/naxes )
    GLOBAL_ADD_COMPONENT( orocos-cartesian ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_CARTESIAN )
