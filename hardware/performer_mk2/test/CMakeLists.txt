DEPENDENT_OPTION( BUILD_PERFORMER_TEST "Build Performer Component Test" OFF "BUILD_PERFORMER;BUILD_TESTS" OFF)

IF ( BUILD_PERFORMER_TEST AND BUILD_COMEDI)

    # This gathers all the .cpp files into the variable 'SRCS'
    # FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( performer main.cpp )

    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/axes ${PROJECT_SOURCE_DIR}/hardware )
    LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/hardware/performer_mk2 ${PROJECT_BINARY_DIR}/taskbrowser)
    PROGRAM_ADD_DEPS( performer orocos-taskbrowser orocos-reporting orocos-performer )

    GLOBAL_ADD_TEST( interface_test interface-test.cpp )

    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/axes ${PROJECT_SOURCE_DIR}/hardware/comedi ${PROJECT_SOURCE_DIR}/hardware/io )
    LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/hardware/performer_mk2 ${PROJECT_BINARY_DIR}/taskbrowser)
    PROGRAM_ADD_DEPS( interface_test orocos-taskbrowser orocos-comedi orocos-io )

ENDIF ( BUILD_PERFORMER_TEST AND BUILD_COMEDI )
