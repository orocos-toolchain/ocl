DEPENDENT_OPTION( BUILD_COMEDI "Build Comedi Component" OFF "BUILD_HARDWARE;COMEDI_FOUND" OFF )

#IF ( BUILD_COMEDI AND NOT COMEDI_FOUND)
#   MESSAGE( FATAL_ERROR "Trying to build comedi drivers, but comedi not found.")
#ENDIF ( BUILD_COMEDI AND NOT COMEDI_FOUND)

IF ( BUILD_COMEDI )

    include (${PROJ_SOURCE_DIR}/config/UseComedi.cmake)

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB DEVSRCS dev/[^.]*.cpp dev/[^.]*.c )
    FILE( GLOB DEVHPPS dev/[^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-comedi ${DEVSRCS} )
    GLOBAL_ADD_INCLUDE( ocl/dev ${DEVHPPS} )

ENDIF ( BUILD_COMEDI )
