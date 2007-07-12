DEPENDENT_OPTION( BUILD_SIM_ROB "Build Simulation Robot Component" OFF  "BUILD_AXES" OFF)

IF( BUILD_SIM_ROB )

  GLOBAL_ADD_COMPONENT(orocos-simrob nAxesVelocityController.cpp)
  INCLUDE_DIRECTORIES(${PROJ_SOURCE_DIR}/hardware/axes)
  TARGET_LINK_LIBRARIES( orocos-simrob orocos-axes)

  GLOBAL_ADD_INCLUDE( ocl nAxesVelocityController.hpp)

  ADD_SUBDIRECTORY( tests )

ENDIF(BUILD_SIM_ROB)
