OPTION( BUILD_TASKBROWSER "Build TaskBrowser" ON )

IF ( BUILD_TASKBROWSER )

    IF(NOT CURSES)
       MESSAGE( FATAL "Can not build TaskBrowser without the ncurses library." )
    ELSE(NOT CURSES)
       OROCOS_PKGCONFIG_LIBS("${CURSES_LIBS}")
       LINK_LIBRARIES( ncurses )

       # This gathers all the .cpp files into the variable 'SRCS'
       FILE( GLOB HPPS [^.]*.hpp )
       FILE( GLOB SRCS [^.]*.cpp )

       GLOBAL_ADD_COMPONENT( orocos-taskbrowser ${SRCS} )
       GLOBAL_ADD_INCLUDE( taskbrowser ${HPPS})
     
       ADD_SUBDIRECTORY( tests )
    ENDIF(NOT CURSES)

ENDIF ( BUILD_TASKBROWSER )
