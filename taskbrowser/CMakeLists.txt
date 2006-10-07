OPTION( BUILD_TASKBROWSER "Build TaskBrowser" ON )

IF ( BUILD_TASKBROWSER )

    INCLUDE( ${CMAKE_ROOT}/Modules/FindCurses.cmake )
    IF( NOT CURSES_FOUND )
       MESSAGE( FATAL "Can not build TaskBrowser without the ncurses library." )
    ELSE(NOT CURSES_FOUND )
       OROCOS_PKGCONFIG_LIBS("{CURSES_LIBS}")
       LINK_LIBRARIES( ncurses )
    ENDIF(NOT CURSES_FOUND)

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB HPPS [^.]*.hpp )
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-taskbrowser ${SRCS} )
    GLOBAL_ADD_INCLUDE( taskbrowser ${HPPS})
     
    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_TASKBROWSER )
