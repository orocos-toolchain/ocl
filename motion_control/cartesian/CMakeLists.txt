DEPENDENT_OPTION( BUILD_CARTESIAN "Build Cartesian Control Components" ON "BUILD_MOTION;KDL_FOUND" OFF)

#Always process subdirs
ADD_SUBDIRECTORY( tests )

IF ( BUILD_CARTESIAN )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/motion_control/naxes/ )
    GLOBAL_ADD_COMPONENT( orocos-cartesian ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})
    COMPONENT_ADD_LIBS(orocos-cartesian ${KDL_LIBS})
    IF(KDLTK_FOUND)
      COMPONENT_ADD_LIBS(orocos-cartesian ${KDLTK_LIBS})
    ENDIF(KDLTK_FOUND)
ENDIF ( BUILD_CARTESIAN )
