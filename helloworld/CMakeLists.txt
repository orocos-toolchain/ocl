DEPENDENT_OPTION( BUILD_HELLOWORLD "Build Helloworld Example" OFF  "BUILD_TASKBROWSER" OFF)

IF ( BUILD_HELLOWORLD )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    ADD_EXECUTABLE( helloworld ${SRCS} )

    TARGET_LINK_LIBRARIES( helloworld ${OROCOS_RTT_LIBS} )
    PROGRAM_ADD_DEPS( helloworld orocos-taskbrowser )
    
ENDIF ( BUILD_HELLOWORLD )
