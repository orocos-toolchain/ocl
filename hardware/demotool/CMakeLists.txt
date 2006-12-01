DEPENDENT_OPTION( BUILD_DEMOTOOL "Build DEMOTOOL Sensor Components" OFF "BUILD_HARDWARE" OFF)

#Always process subdirs
ADD_SUBDIRECTORY( tests )

IF ( BUILD_DEMOTOOL )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-demotool ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

ENDIF ( BUILD_DEMOTOOL )
