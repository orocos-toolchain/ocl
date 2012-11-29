
option(BUILD_LUA_RTT "Build Lua RTT bindings" ON)
cmake_dependent_option(BUILD_LUA_RTT_DYNAMIC_MODULES "Build dynamic loadable Lua plugins for rtt and deployer (experimental!)" OFF "BUILD_LUA_RTT" OFF)

cmake_dependent_option(BUILD_LUA_RTT_TLSF "Build TLSF enabled Lua version (experimental!)" ON "BUILD_LUA_RTT" OFF)
cmake_dependent_option(BUILD_LUA_RTT_CORBA "Build CORBA enabled Lua version (experimental!)" ON "BUILD_LUA_RTT" OFF)
cmake_dependent_option(LUA_RTT_TYPEINFO_CACHING "Let Lua RTT bindings do Typeinfo Caching" ON "BUILD_LUA_RTT" OFF)

if(BUILD_LUA_RTT)

  find_package(Lua51)

  if(NOT LUA51_FOUND)

    message(WARNING "Can not build LuaComponent without Lua library. On debian-like systems try 'sudo apt-get install liblua5.1-0-dev'.")

  else(NOT LUA51_FOUND)

    if ( BUILD_RTALLOC )
      # default to 512 kilobytes - see minimum size warning in deployer-funcs.cpp
      set(ORO_DEFAULT_RTALLOC_SIZE 524288 CACHE STRING "Real-time memory pool size (bytes) for operations and logging.")
      add_definitions("-DORO_BUILD_RTALLOC -DORO_DEFAULT_RTALLOC_SIZE=${ORO_DEFAULT_RTALLOC_SIZE}")
    else ( BUILD_RTALLOC )
      add_definitions("-DORO_DEFAULT_RTALLOC_SIZE=0")
    endif ( BUILD_RTALLOC )

    if ( BUILD_LOGGING AND LOG4CPP_FOUND )
      include_directories( "${LOG4CPP_INCLUDE_DIRS}" )
      link_directories( "${LOG4CPP_LIBRARY_DIRS}" )
      list(APPEND EXTRA_LIBRARIES "${LOG4CPP_LIBRARY}")
      list(APPEND EXTRA_DEPS "orocos-ocl-log4cpp")
      add_definitions("-DORO_BUILD_LOGGING")
    endif ( BUILD_LOGGING AND LOG4CPP_FOUND )

    # compiling lua component and rttlua interpreter
    include_directories(${LUA_INCLUDE_DIR})

    add_library(lua-repl STATIC lua-repl.c)
    target_link_libraries(lua-repl ${LUA_LIBRARIES})
    set_target_properties(lua-repl PROPERTIES COMPILE_FLAGS "-DOROCOS_TARGET=${OROCOS_TARGET}")

    if(LUA_RTT_TYPEINFO_CACHING)
      add_definitions("-DTYPEINFO_CACHING")
    endif(LUA_RTT_TYPEINFO_CACHING)

    orocos_component(orocos-ocl-lua rtt.cpp LuaComponent.cpp )
    orocos_executable(rttlua rtt.cpp LuaComponent.cpp)
    program_add_deps(rttlua lua-repl orocos-ocl-deployment ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES} ${EXTRA_DEPS} ${EXTRA_LIBRARIES} )
    target_link_libraries(orocos-ocl-lua ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES} )
    set_target_properties(orocos-ocl-lua PROPERTIES COMPILE_FLAGS "-DOCL_COMPONENT_ONLY")
    install(TARGETS rttlua RUNTIME DESTINATION bin)
    install(PROGRAMS ../bin/rttlua DESTINATION bin) # The shell script

    install(FILES modules/rttlib.lua modules/ansicolors.lua modules/utils.lua modules/rttros.lua DESTINATION share/lua/5.1/)

    # Handle curses/readline
    if(NOT CURSES AND NOT READLINE)
      message(STATUS "Building without readline/curses support. If you want support for this try someting like 'sudo aptitude install libncurses5-dev libreadline5-dev'")
      set(LUA_REPL_FLAGS "-DLUA_ANSI")
    else(NOT CURSES AND NOT READLINE)
      set(LUA_REPL_FLAGS "-DLUA_USE_LINUX")
      target_link_libraries(lua-repl "readline" ${CURSES_IMPL} "dl" "history" )
    endif(NOT CURSES AND NOT READLINE)

    set_target_properties(lua-repl PROPERTIES COMPILE_FLAGS "${LUA_REPL_FLAGS}")

    # CORBA version
    if(BUILD_LUA_RTT_CORBA AND OROCOS-RTT_CORBA_FOUND)

      orocos_executable(rttlua-corba rtt.cpp LuaComponent.cpp)
      program_add_deps(rttlua-corba lua-repl orocos-ocl-deployment ${LUA_LIBRARIES} ${OROCOS-RTT_CORBA_LIBRARIES} orocos-ocl-deployment-corba ${EXTRA_DEPS} ${EXTRA_LIBRARIES} )
      set_target_properties(rttlua-corba PROPERTIES COMPILE_FLAGS "-DLUA_RTT_CORBA")
      install(TARGETS rttlua-corba RUNTIME DESTINATION bin)
      install(PROGRAMS ../bin/rttlua DESTINATION bin RENAME rttlua-corba) # The shell script

    endif(BUILD_LUA_RTT_CORBA AND OROCOS-RTT_CORBA_FOUND)

    # TLSF version
    if(BUILD_LUA_RTT_TLSF)

      orocos_component(orocos-ocl-lua-tlsf rtt.cpp LuaComponent.cpp )
      orocos_executable(rttlua-tlsf rtt.cpp LuaComponent.cpp)
      add_library(lua-repl-tlsf STATIC lua-repl.c)
      target_link_libraries(lua-repl-tlsf "readline" ${CURSES_IMPL} "dl" "history" ${LUA_LIBRARIES})
      set_target_properties(lua-repl-tlsf PROPERTIES COMPILE_FLAGS "-DOROCOS_TARGET=${OROCOS_TARGET} -DLUA_RTT_TLSF ${LUA_REPL_FLAGS}")

      add_library(tlsf_rtt STATIC tlsf.c tlsf_rtt.c)
      set_target_properties(tlsf_rtt PROPERTIES COMPILE_FLAGS "-DTLSF_STATISTIC -DCHECK_DOUBLE_FREE -fPIC -Wextra -Wall -Werror")

      set_target_properties(orocos-ocl-lua-tlsf PROPERTIES COMPILE_FLAGS "-DOCL_COMPONENT_ONLY -DLUA_RTT_TLSF")
      set_target_properties(rttlua-tlsf PROPERTIES COMPILE_FLAGS "-DLUA_RTT_TLSF")

      program_add_deps(rttlua-tlsf tlsf_rtt lua-repl-tlsf orocos-ocl-deployment ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES} ${EXTRA_DEPS} ${EXTRA_LIBRARIES} )
      target_link_libraries(orocos-ocl-lua-tlsf tlsf_rtt ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES})

      install(TARGETS rttlua-tlsf RUNTIME DESTINATION bin)
      install(PROGRAMS ../bin/rttlua-tlsf DESTINATION bin) # The shell script
      install(FILES modules/tlsf_ext.lua DESTINATION share/lua/5.1/)
    endif(BUILD_LUA_RTT_TLSF)

    add_subdirectory( plugins )
    add_subdirectory( testing )

    orocos_generate_package()

    if(BUILD_LUA_RTT_DYNAMIC_MODULES)

      #  compiling rtt / deployer as lua modules
      add_library(deployer SHARED deployer.cpp)
      add_library(rtt SHARED rtt.cpp)

      target_link_libraries(deployer ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES} orocos-ocl-deployment)
      target_link_libraries(rtt ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES})

      # need next line?
      set_target_properties(deployer PROPERTIES PREFIX "")
      set_target_properties(rtt PROPERTIES PREFIX "")

    endif(BUILD_LUA_RTT_DYNAMIC_MODULES)

  endif(NOT LUA51_FOUND)

endif(BUILD_LUA_RTT)
