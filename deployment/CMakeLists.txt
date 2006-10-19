OPTION( BUILD_DEPLOYMENT "Build Deployment" ON )

IF ( BUILD_DEPLOYMENT )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB HPPS [^.]*.hpp )
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-deployment ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})
     
    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_DEPLOYMENT )
