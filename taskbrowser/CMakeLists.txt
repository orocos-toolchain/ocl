OPTION( BUILD_TASKBROWSER "Build TaskBrowser" ON )
OPTION( TASKBROWSER_EDITLINE "Use editline instead of readline for TaskBrowser." OFF)

IF ( BUILD_TASKBROWSER )
    IF ( TASKBROWSER_EDITLINE AND NOT EDITLINE)
      MESSAGE( FATAL_ERROR "Can't build TaskBrowser with editline support since it's not found (see TASKBROWSER_EDITLINE option).")
    ENDIF ( TASKBROWSER_EDITLINE AND NOT EDITLINE)

    #Try to use editline in !GPL or user option:
    IF (NO_GPL OR TASKBROWSER_EDITLINE)
      SET(USE_EDITLINE 1)
    ENDIF (NO_GPL OR TASKBROWSER_EDITLINE)

    IF ( USE_EDITLINE )
      MESSAGE( "Building TaskBrowser with editline support." )
    ELSE( USE_EDITLINE )
      IF (READLINE)
	SET(USE_READLINE 1)
	MESSAGE( "Building TaskBrowser with readline support." )
      ELSE (READLINE)
	MESSAGE( "Building TaskBrowser without readline/editline support!" )
      ENDIF (READLINE)
    ENDIF( USE_EDITLINE )

    # We just signal that readline is available. The code itself
    # will check this flag + the NO_GPL flag.
    IF( USE_READLINE )
      ADD_DEFINITIONS(-DHAS_READLINE)
    ENDIF( USE_READLINE )
    IF( USE_EDITLINE )
      ADD_DEFINITIONS(-DHAS_EDITLINE)
    ENDIF( USE_EDITLINE )
      
    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB HPPS [^.]*.hpp )
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-taskbrowser ${SRCS} INSTALL lib)
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

    # Readline support:
    IF (USE_READLINE)
      # MSVC builds link statically with the readline.lib file
      if (MSVC)
	# Should this flag be set for mingw too ??
	MESSAGE("Taskbrowser NOTE: you need to have a static readline build from a readline MSVC solution !")
	ADD_DEFINITIONS( /DREADLINE_STATIC )
      endif()

      # readline only depends on curses on unixes, not on mingw/msvc
      if (UNIX)
        OROCOS_PKGCONFIG_LIBS(ncurses readline)
        INCLUDE_DIRECTORIES( ${READLINE_INCLUDE_DIR} ${CURSES_INCLUDE_DIR} )
        COMPONENT_ADD_LIBS( orocos-taskbrowser ${READLINE_LIBRARY} ${CURSES_LIBRARY} )
      else(UNIX)
        OROCOS_PKGCONFIG_LIBS(readline)
        INCLUDE_DIRECTORIES( ${READLINE_INCLUDE_DIR} )
        COMPONENT_ADD_LIBS( orocos-taskbrowser ${READLINE_LIBRARY} )
      endif(UNIX)

    ENDIF (USE_READLINE)

    # Editline support:
    IF (USE_EDITLINE)
      if (UNIX)
        OROCOS_PKGCONFIG_LIBS(editline)
        INCLUDE_DIRECTORIES( ${EDITLINE_INCLUDE_DIR} ${CURSES_INCLUDE_DIR} )
        COMPONENT_ADD_LIBS( orocos-taskbrowser ${EDITLINE_LIBRARY} ${CURSES_LIBRARY} )
      else(UNIX)
        OROCOS_PKGCONFIG_LIBS(editline)
        INCLUDE_DIRECTORIES( ${EDITLINE_INCLUDE_DIR} )
        COMPONENT_ADD_LIBS( orocos-taskbrowser ${EDITLINE_LIBRARY} )
      endif(UNIX)
    ENDIF (USE_EDITLINE)

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_TASKBROWSER )
