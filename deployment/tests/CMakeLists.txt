
DEPENDENT_OPTION( BUILD_DEPLOYMENT_TEST "Build Deployment Test" ON "BUILD_DEPLOYMENT;BUILD_TESTS" OFF)

IF ( BUILD_DEPLOYMENT_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_TEST( deploy ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    TARGET_LINK_LIBRARIES( deploy orocos-taskbrowser orocos-deployment ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_DEPLOYMENT_TEST )
