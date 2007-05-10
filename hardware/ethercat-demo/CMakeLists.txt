DEPENDENT_OPTION( BUILD_ETHERCATDEMO "Build Ethercatdemo Components" OFF  "BUILD_AXES" OFF)

#Always process subdirs
ADD_SUBDIRECTORY( tests )
ADD_SUBDIRECTORY( dev )

IF ( BUILD_ETHERCATDEMO )

  # This gathers all the .cpp files into the variable 'SRCS'
  FILE( GLOB SRCS EthercatIO.cpp )
  FILE( GLOB HPPS EthercatIO.hpp )
  
  GLOBAL_ADD_COMPONENT( orocos-ethercatdemo ${SRCS} )
  INCLUDE_DIRECTORIES(${PROJ_SOURCE_DIR}/hardware/axes /hardware/ethercat-demo /usr/local/include/ethercat/arch-RTnet)
  TARGET_LINK_LIBRARIES( orocos-ethercatdemo orocos-axes eml rtdm)
  
  GLOBAL_ADD_INCLUDE( ocl ${HPPS})
  
ENDIF ( BUILD_ETHERCATDEMO )
