DEPENDENT_OPTION( BUILD_KUKA "Build Kuka Components" OFF  "BUILD_AXES;KDL_FOUND" OFF)

#Always process subdirs
ADD_SUBDIRECTORY( tests )

IF ( BUILD_KUKA )

  # This gathers all the .cpp files into the variable 'SRCS'
  FILE( GLOB SRCS [^.]*.cpp )
  FILE( GLOB HPPS [^.]*.hpp )
  
  GLOBAL_ADD_COMPONENT( orocos-kuka ${SRCS} )
  INCLUDE_DIRECTORIES(${PROJ_SOURCE_DIR}/hardware/axes)
  COMPONENT_ADD_DEPS( orocos-kuka orocos-axes orocos-ocl-common)
  COMPONENT_ADD_LIBS( orocos-kuka orocos-kdl)
  
  IF ( BUILD_COMEDI )
    INCLUDE_DIRECTORIES(${PROJ_SOURCE_DIR}/hardware/comedi)
    COMPONENT_ADD_DEPS( orocos-kuka orocos-comedi )
  ENDIF ( BUILD_COMEDI )
  
  IF ( BUILD_APCI )
    INCLUDE_DIRECTORIES(${PROJ_SOURCE_DIR}/hardware/apci )
    COMPONENT_ADD_DEPS( orocos-kuka orocos-apci )
  ENDIF ( BUILD_APCI )
  
  GLOBAL_ADD_INCLUDE( ocl ${HPPS})
  
ENDIF ( BUILD_KUKA )
