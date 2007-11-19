DEPENDENT_OPTION( BUILD_SIM_ROB_TEST "Build robot simulation component test" OFF "BUILD_SIM_ROB;BUILD_TESTS;BUILD_TASKBROWSER" OFF)

IF (BUILD_SIM_ROB_TEST)
  GLOBAL_ADD_TEST(robsim main.cpp)
  INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/axes)
  LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/hardware/robotsimulation )
  PROGRAM_ADD_DEPS( robsim orocos-taskbrowser orocos-simrob )
ENDIF(BUILD_SIM_ROB_TEST)