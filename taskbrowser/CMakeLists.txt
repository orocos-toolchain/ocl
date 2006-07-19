OPTION( BUILD_TASKBROWSER "Build TaskBrowser" ON )

IF ( BUILD_TASKBROWSER )

    INCLUDE( ${PROJ_SOURCE_DIR}/config/component_rules.cmake )
    #INCLUDE( ${PROJ_SOURCE_DIR}/config/include_orocosrtt.cmake )

    # keyboard driver with libncurses
#     OPTIONAL_SUB_LIBRARY(
#         "Ncurses driver for TaskBrowser"
#         taskbrowser-ncurses
#         OrcaTaskbrowserKeyboardNcurses
#         KBD_NCURSES_LIBS
#         CURSES_FOUND
#         HAVE_KEYBOARD_NCURSES_DRIVER
#         ncurses
#     )
    
#     IF ( NOT HAVE_KEYBOARD_NCURSES_DRIVER )
#        MESSAGE("    WARNING: No TaskBrowser  can be built")
#     ENDIF ( NOT HAVE_KEYBOARD_NCURSES_DRIVER )
    
    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS *.cpp )

    ADD_LIBRARY( taskbrowser SHARED ${SRCS} )
    INSTALL_TARGETS( /lib taskbrowser )

    ADD_SUBDIRECTORY( tests )
ENDIF ( BUILD_TASKBROWSER )
