CMAKE_DEPENDENT_OPTION( BUILD_HELLOWORLD "Build Helloworld Example" ON  "BUILD_TASKBROWSER" OFF)

IF ( BUILD_HELLOWORLD )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    ADD_EXECUTABLE( helloworld ${SRCS} )
    orocos_component( orocos-helloworld ${SRCS} )
    SET_TARGET_PROPERTIES( orocos-helloworld
      PROPERTIES COMPILE_FLAGS "-DOCL_COMPONENT_ONLY" )

    TARGET_LINK_LIBRARIES( helloworld ${OROCOS-RTT_LIBRARIES} orocos-taskbrowser)
    
ENDIF ( BUILD_HELLOWORLD )
