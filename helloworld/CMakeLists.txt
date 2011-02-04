CMAKE_DEPENDENT_OPTION( BUILD_HELLOWORLD "Build Helloworld Example" ON  "BUILD_TASKBROWSER" OFF)

IF ( BUILD_HELLOWORLD )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    ADD_EXECUTABLE( helloworld ${SRCS} )
    orocos_component( orocos-ocl-helloworld ${SRCS} )
    SET_TARGET_PROPERTIES( orocos-ocl-helloworld
      PROPERTIES COMPILE_FLAGS "-DOCL_COMPONENT_ONLY" )

    TARGET_LINK_LIBRARIES( helloworld ${OROCOS-RTT_LIBRARIES} orocos-ocl-taskbrowser)
    SET_TARGET_PROPERTIES( helloworld PROPERTIES
      DEFINE_SYMBOL OCL_DLL_EXPORT)
ENDIF ( BUILD_HELLOWORLD )
