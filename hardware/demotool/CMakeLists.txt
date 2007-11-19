DEPENDENT_OPTION( BUILD_DEMOTOOL "Build DEMOTOOL Sensor Components" OFF "BUILD_HARDWARE;BUILD_WRENCH;BUILD_KRYPTON" OFF)

#Always process subdirs
ADD_SUBDIRECTORY( tests )

IF ( BUILD_DEMOTOOL )

  # This gathers all the .cpp files into the variable 'SRCS'
  FILE( GLOB SRCS [^.]*.cpp )
  FILE( GLOB HPPS [^.]*.hpp )
  
  GLOBAL_ADD_COMPONENT( orocos-demotool ${SRCS} )
  GLOBAL_ADD_INCLUDE( ocl ${HPPS})

  COMPONENT_ADD_LIBS( orocos-demotool wrapper_matrix ltir orocos-bfl orocos-kdl )
  COMPONENT_ADD_DEPS( orocos-demotool orocos-wrench orocos-krypton )

ENDIF ( BUILD_DEMOTOOL )
