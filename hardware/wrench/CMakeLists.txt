DEPENDENT_OPTION( BUILD_WRENCH "Build WrenchSensor Components" OFF "BUILD_HARDWARE" OFF)

ADD_SUBDIRECTORY( tests )

IF ( BUILD_WRENCH )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-wrench ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS} )
    IF (OS_LXRT)
      GLOBAL_ADD_INCLUDE( ocl/drivers drivers/jr3_lxrt_common.h )
    ENDIF (OS_LXRT)

ENDIF ( BUILD_WRENCH )
