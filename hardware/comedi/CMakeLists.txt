OPTION( BUILD_COMEDI "Build Comedi Component" ON )

IF ( BUILD_COMEDI )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-comedi ${SRCS} )
    GLOBAL_ADD_INCLUDE( hardware/comedi ${HPPS} )
    #GLOBAL_ADD_INCLUDE( hardware/comedi/driver/ driver/jr3_lxrt_common.h )

    #ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_COMEDI )
