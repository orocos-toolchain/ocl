DEPENDENT_OPTION( BUILD_AXES "Build Axes Component" ON "BUILD_HARDWARE" OFF)

IF ( BUILD_AXES )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS AxesComponent.cpp )
    FILE( GLOB HPPS AxesComponent.hpp )

    GLOBAL_ADD_INCLUDE( ocl ${HPPS} )

    FILE( GLOB DEVSRCS dev/[^.]*.cpp )
    FILE( GLOB DEVHPPS dev/[^.]*.hpp )
    GLOBAL_ADD_COMPONENT( orocos-axes ${DEVSRCS} ${SRCS})
    GLOBAL_ADD_INCLUDE( ocl/dev ${DEVHPPS} )

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_AXES )
