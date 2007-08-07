DEPENDENT_OPTION( BUILD_STAUBLI "Build Staubli Components" OFF  "BUILD_AXES;KDL_FOUND" OFF)

#Always process subdirs
#ADD_SUBDIRECTORY( tests )

IF ( BUILD_STAUBLI )

  # This gathers all the .cpp files into the variable 'SRCS'
  FILE( GLOB SRCS [^.]*.cpp )
  FILE( GLOB HPPS [^.]*.hpp )
  
  GLOBAL_ADD_COMPONENT( orocos-staubli ${SRCS} )
  INCLUDE_DIRECTORIES(${PROJ_SOURCE_DIR}/hardware/axes)
  TARGET_LINK_LIBRARIES( orocos-staubli orocos-axes orocos-kdl)
  
  IF ( BUILD_COMEDI )
    INCLUDE_DIRECTORIES(${PROJ_SOURCE_DIR}/hardware/comedi)
    TARGET_LINK_LIBRARIES( orocos-staubli orocos-comedi )
  ENDIF ( BUILD_COMEDI )
  
  GLOBAL_ADD_INCLUDE( ocl ${HPPS})
  
ENDIF ( BUILD_STAUBLI )
