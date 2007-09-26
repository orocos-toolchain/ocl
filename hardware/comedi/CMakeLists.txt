DEPENDENT_OPTION( BUILD_COMEDI "Build Comedi Component" OFF "BUILD_HARDWARE;COMEDI_FOUND" OFF )

#IF ( BUILD_COMEDI AND NOT COMEDI_FOUND)
#   MESSAGE( FATAL_ERROR "Trying to build comedi drivers, but comedi not found.")
#ENDIF ( BUILD_COMEDI AND NOT COMEDI_FOUND)

IF ( BUILD_COMEDI )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB DEVSRCS dev/[^.]*.cpp )
    FILE( GLOB DEVHPPS dev/[^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-comedi ${DEVSRCS} )
    GLOBAL_ADD_INCLUDE( ocl/dev ${DEVHPPS} )

    SET_TARGET_PROPERTIES( orocos-comedi PROPERTIES COMPILE_FLAGS "-I${COMEDI_INCLUDE_DIRS}")
    
    #ADD_SUBDIRECTORY( tests )

    IF( OS_GNULINUX )
    TARGET_LINK_LIBRARIES(orocos-comedi comedi)
    OROCOS_PKGCONFIG_LIBS("-lcomedi")
    ENDIF( OS_GNULINUX )

    IF( OS_LXRT )
    TARGET_LINK_LIBRARIES(orocos-comedi kcomedilxrt)
    OROCOS_PKGCONFIG_LIBS("-lkcomedilxrt")
    ENDIF( OS_LXRT )

ENDIF ( BUILD_COMEDI )
