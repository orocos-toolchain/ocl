OPTION( BUILD_TASKBROWSER "Build TaskBrowser" ON )

IF ( BUILD_TASKBROWSER )

    IF(NOT CURSES AND NOT READLINE)
       MESSAGE( FATAL_ERROR "Can not build TaskBrowser without the
       ncurses/readline library. Try 'sudo apt-get install libncurses5-dev libreadline5-dev' or
       disable the TaskBrowser" )
    ELSE(NOT CURSES AND NOT READLINE)
       OROCOS_PKGCONFIG_LIBS("-lncurses -lreadline")

       # This gathers all the .cpp files into the variable 'SRCS'
       FILE( GLOB HPPS [^.]*.hpp )
       FILE( GLOB SRCS [^.]*.cpp )

       GLOBAL_ADD_COMPONENT( orocos-taskbrowser ${SRCS} )
       GLOBAL_ADD_INCLUDE( ocl ${HPPS})
     
       TARGET_LINK_LIBRARIES(orocos-taskbrowser ncurses readline )

       ADD_SUBDIRECTORY( tests )
    ENDIF(NOT CURSES AND NOT READLINE)

ENDIF ( BUILD_TASKBROWSER )
