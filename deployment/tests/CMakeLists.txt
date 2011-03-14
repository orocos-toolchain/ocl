
CMAKE_DEPENDENT_OPTION( BUILD_DEPLOYMENT_TEST "Build Deployment Test" ON "BUILD_DEPLOYMENT;BUILD_TASKBROWSER;BUILD_TESTS" OFF)

IF ( BUILD_DEPLOYMENT_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_TEST( deploy ${SRCS} )
    PROGRAM_ADD_DEPS( deploy orocos-ocl-taskbrowser orocos-ocl-deployment )

    # Copy this file to build dir.
    TEST_USES_FILE( ComponentA.cpf )
    TEST_USES_FILE( ComponentB.cpf )
    TEST_USES_FILE( deployment.cpf )

ENDIF ( BUILD_DEPLOYMENT_TEST )
