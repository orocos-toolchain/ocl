
option(BUILD_LUA_RTT "Build Lua RTT bindings" OFF)
cmake_dependent_option(BUILD_LUA_RTT_DYNAMIC_MODULES "Build dynamic loadable Lua plugins for rtt and deployer (experimental!)" OFF "BUILD_LUA_RTT" OFF)

if(BUILD_LUA_RTT)

  find_package(Lua51 REQUIRED)

  if(NOT LUA51_FOUND)

  message(FATAL_ERROR "Can not build LuaComponent without lua
      libraries. On debian-like system you might want to try something
      like 'sudo apt-get install liblua5.1-0-dev' or disable 
      Lua-RTT completely.")

  else(NOT LUA51_FOUND)

    # compiling lua component and rttlua interpreter
    include_directories(${LUA_INCLUDE_DIR})

    add_library(lua-repl STATIC lua-repl.c)
    target_link_libraries(lua-repl "ncurses" "readline" "dl" "history" ${LUA_LIBRARIES})
    set_target_properties(lua-repl PROPERTIES COMPILE_FLAGS "-DOROCOS_TARGET=${OROCOS_TARGET}")

    orocos_component(orocos-ocl-lua rtt.cpp LuaComponent.cpp )
    orocos_executable(rttlua rtt.cpp LuaComponent.cpp)
    set_target_properties(orocos-ocl-lua PROPERTIES COMPILE_FLAGS "-DOCL_COMPONENT_ONLY")
    program_add_deps(rttlua orocos-ocl-deployment)
    install(TARGETS rttlua RUNTIME DESTINATION bin)
    install(FILES modules/rttlib.lua modules/ansicolors.lua modules/utils.lua DESTINATION usr/share/lua/5.1/)
    if(NOT CURSES AND NOT READLINE)
       message(STATUS "Building without readline/curses support. If you
	 want support for this try someting like 'sudo aptitude install
	 libncurses5-dev libreadline5-dev'")

       set_target_properties(lua-repl PROPERTIES COMPILE_FLAGS "-DLUA_ANSI")

     else(NOT CURSES AND NOT READLINE)

       set_target_properties(lua-repl PROPERTIES COMPILE_FLAGS "-DLUA_USE_LINUX")

     endif(NOT CURSES AND NOT READLINE)

     program_add_deps(rttlua lua-repl ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES})
     target_link_libraries(orocos-ocl-lua ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES})

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

