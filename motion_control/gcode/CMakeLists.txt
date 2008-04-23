DEPENDENT_OPTION( BUILD_GCODE "Build Gcode Receiver" ON "BUILD_MOTION;KDL_FOUND" OFF)

IF ( BUILD_GCODE )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/motion_control/gcode/ )
    GLOBAL_ADD_COMPONENT( orocos-gcode ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})
    COMPONENT_ADD_LIBS(orocos-gcode ${KDL_LIBS})
    IF(KDLTK_FOUND)
      COMPONENT_ADD_LIBS(orocos-gcode ${KDLTK_LIBS})
    ENDIF(KDLTK_FOUND)
ENDIF ( BUILD_GCODE )
