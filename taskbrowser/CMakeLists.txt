OPTION( BUILD_TASKBROWSER "Build TaskBrowser" ON )

IF ( BUILD_TASKBROWSER )

    IF( NOT READLINE AND NOT NO_GPL)
       MESSAGE( FATAL_ERROR "Can not build TaskBrowser without the
       ncurses/readline library. Try 'sudo apt-get install libncurses5-dev libreadline5-dev' or
       turn on NO_GPL or disable the TaskBrowser Component completely." )
    ELSE( NOT READLINE AND NOT NO_GPL)
       # This gathers all the .cpp files into the variable 'SRCS'
       FILE( GLOB HPPS [^.]*.hpp )
       FILE( GLOB SRCS [^.]*.cpp )

       GLOBAL_ADD_COMPONENT( orocos-taskbrowser ${SRCS} )
       GLOBAL_ADD_INCLUDE( ocl ${HPPS})

       IF (NOT NO_GPL)
	 OROCOS_PKGCONFIG_LIBS(ncurses readline)
	 INCLUDE_DIRECTORIES(${READLINE_H})
	 COMPONENT_ADD_LIBS( orocos-taskbrowser ${READLINE_LIBRARY} )
       ENDIF (NOT NO_GPL)

       COMPONENT_ADD_DEPS( orocos-taskbrowser )

       ADD_SUBDIRECTORY( tests )
    ENDIF(NOT READLINE AND NOT NO_GPL)

ENDIF ( BUILD_TASKBROWSER )
