OPTION( BUILD_TASKBROWSER "Build TaskBrowser" ON )

IF ( BUILD_TASKBROWSER )

    find_package(RTTPlugin REQUIRED rtt-scripting)

    IF( NOT READLINE OR NO_GPL)
       MESSAGE( "Building TaskBrowser without readline support!" )
    ENDIF(NOT READLINE OR NO_GPL)

    # We just signal that readline is available. The code itself
    # will check this flag + the NO_GPL flag.
    IF( READLINE )
      ADD_DEFINITIONS(-DHAS_READLINE)
    ENDIF( READLINE )
      
    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB HPPS [^.]*.hpp )
    FILE( GLOB SRCS [^.]*.cpp )

    orocos_library( orocos-taskbrowser ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

    # Readline support:
    IF (READLINE AND NOT NO_GPL)
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
        TARGET_LINK_LIBRARIES( orocos-taskbrowser ${READLINE_LIBRARY} ${CURSES_LIBRARY} )
      else(UNIX)
        OROCOS_PKGCONFIG_LIBS(readline)
        INCLUDE_DIRECTORIES( ${READLINE_INCLUDE_DIR} )
        TARGET_LINK_LIBRARIES( orocos-taskbrowser ${READLINE_LIBRARY} )
      endif(UNIX)

    ENDIF (READLINE AND NOT NO_GPL)

    TARGET_LINK_LIBRARIES( orocos-taskbrowser ${RTT_PLUGIN_rtt-scripting_LIBRARIES} )
    target_link_libraries( orocos-taskbrowser LINK_INTERFACE_LIBRARIES )

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_TASKBROWSER )
