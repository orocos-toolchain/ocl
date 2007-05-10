DEPENDENT_OPTION( BUILD_ETHERCATDEMO_TEST "Build Ethercatdemo Components Test" OFF "BUILD_ETHERCATDEMO;BUILD_TESTS" OFF)

IF ( BUILD_ETHERCATDEMO_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    # FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( ethercatdemo main.cpp )

    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/axes ${PROJECT_SOURCE_DIR}/hardware/ethercat-demo)
    LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/hardware/ethercat-demo )
    TARGET_LINK_LIBRARIES( ethercatdemo orocos-taskbrowser orocos-reporting orocos-ethercatdemo orocos-io ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_ETHERCATDEMO_TEST )
