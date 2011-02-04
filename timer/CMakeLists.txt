OPTION( BUILD_TIMER "Build Timer Component" ON )

IF ( BUILD_TIMER )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB HPPS [^.]*.hpp )
    FILE( GLOB SRCS [^.]*.cpp )

    orocos_component( orocos-ocl-timer ${SRCS} )
    SET_TARGET_PROPERTIES( orocos-ocl-timer PROPERTIES
      DEFINE_SYMBOL OCL_DLL_EXPORT)
    orocos_install_headers( ${HPPS} INSTALL include/orocos/ocl )
    orocos_generate_package()
     
    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_TIMER )
