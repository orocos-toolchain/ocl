DEPENDENT_OPTION( BUILD_COMEDI "Build Comedi Component" OFF "BUILD_HARDWARE" OFF )

IF ( BUILD_COMEDI AND COMEDI_FOUND)

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB DEVSRCS dev/[^.]*.cpp )
    FILE( GLOB DEVHPPS dev/[^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-comedi ${DEVSRCS} )
    GLOBAL_ADD_INCLUDE( ocl/dev ${DEVHPPS} )

    #ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_COMEDI AND COMEDI_FOUND)
