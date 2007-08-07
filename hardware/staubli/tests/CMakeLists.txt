DEPENDENT_OPTION( BUILD_STAUBLI_TEST "Build Staubli Components Test" OFF "BUILD_STAUBLI;BUILD_TESTS" OFF)

IF ( BUILD_STAUBLI_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    # FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( staubli main.cpp )

    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/hardware/axes ${PROJECT_SOURCE_DIR}/hardware/comedi)
    LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/hardware/staubli )
    TARGET_LINK_LIBRARIES( staubli orocos-taskbrowser orocos-reporting orocos-staubli ${OROCOS_RTT_LIBS})


ENDIF ( BUILD_STAUBLI_TEST )
