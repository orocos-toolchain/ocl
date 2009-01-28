DEPENDENT_OPTION( BUILD_GRIPPER "Build Gripper Component" OFF "BUILD_HARDWARE;BUILD_COMEDI" OFF)

IF ( BUILD_GRIPPER )
  
  GLOBAL_ADD_COMPONENT( orocos-gripper Gripper.cpp )
  GLOBAL_ADD_INCLUDE( ocl Gripper.hpp)

  INCLUDE (${PROJ_SOURCE_DIR}/config/UseComedi.cmake)
  INCLUDE_DIRECTORIES(${PROJ_SOURCE_DIR}/hardware/comedi)
  COMPONENT_ADD_DEPS( orocos-gripper orocos-comedi )

ENDIF ( BUILD_GRIPPER )
