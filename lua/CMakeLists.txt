
option(BUILD_LUA_RTT "Build Lua RTT bindings" ON)
cmake_dependent_option(BUILD_LUA_RTT_DYNAMIC_MODULES "Build dynamic loadable Lua plugins for rtt and deployer (experimental!)" OFF "BUILD_LUA_RTT" OFF)

cmake_dependent_option(BUILD_LUA_RTT_TLSF "Build TLSF enabled Lua version (experimental!)" ON "BUILD_LUA_RTT" OFF)

if(BUILD_LUA_RTT)

  find_package(Lua51)

  if(NOT LUA51_FOUND)

    message(WARNING "Can not build LuaComponent without Lua library. On debian-like systems try 'sudo apt-get install liblua5.1-0-dev'.")

  else(NOT LUA51_FOUND)

    # compiling lua component and rttlua interpreter
    include_directories(${LUA_INCLUDE_DIR})

    add_library(lua-repl STATIC lua-repl.c)
    target_link_libraries(lua-repl "ncurses" "readline" "dl" "history" ${LUA_LIBRARIES})
    set_target_properties(lua-repl PROPERTIES COMPILE_FLAGS "-DOROCOS_TARGET=${OROCOS_TARGET}")

    orocos_component(orocos-ocl-lua rtt.cpp LuaComponent.cpp )
    orocos_executable(rttlua rtt.cpp LuaComponent.cpp)
    program_add_deps(rttlua orocos-ocl-deployment)
    install(TARGETS rttlua RUNTIME DESTINATION bin)
    install(FILES modules/rttlib.lua modules/ansicolors.lua modules/utils.lua modules/rttros.lua DESTINATION usr/share/lua/5.1/)

    # No curses/readline
    if(NOT CURSES AND NOT READLINE)
      message(STATUS "Building without readline/curses support. If you
	 want support for this try someting like 'sudo aptitude install
	 libncurses5-dev libreadline5-dev'")

       set_target_properties(lua-repl PROPERTIES COMPILE_FLAGS "-DLUA_ANSI")
     else(NOT CURSES AND NOT READLINE)
       set_target_properties(lua-repl PROPERTIES COMPILE_FLAGS "-DLUA_USE_LINUX")
     endif(NOT CURSES AND NOT READLINE)

     # TLSF version
     if(BUILD_LUA_RTT_TLSF)
       # add "(TLSF)" to boilerplate
       get_target_property(LUA_REPL_FLAGS lua-repl COMPILE_FLAGS)
       set(LUA_REPL_FLAGS "${LUA_REPL_FLAGS} -DLUA_RTT_TLSF")
       set_target_properties(lua-repl PROPERTIES COMPILE_FLAGS "${LUA_REPL_FLAGS}")

       add_library(tlsf_rtt STATIC tlsf.c tlsf_rtt.c)
       set_target_properties(tlsf_rtt PROPERTIES COMPILE_FLAGS "-DTLSF_STATISTIC -DCHECK_DOUBLE_FREE -fPIC -Wextra -Wall -Werror")
       set_target_properties(orocos-ocl-lua PROPERTIES COMPILE_FLAGS "-DOCL_COMPONENT_ONLY -DLUA_RTT_TLSF")
       set_target_properties(rttlua PROPERTIES COMPILE_FLAGS "-DLUA_RTT_TLSF")

       program_add_deps(rttlua lua-repl tlsf_rtt ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES})
       target_link_libraries(orocos-ocl-lua tlsf_rtt ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES})

     else(BUILD_LUA_RTT_TLSF)
       program_add_deps(rttlua lua-repl ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES})
       target_link_libraries(orocos-ocl-lua ${LUA_LIBRARIES} ${OROCOS-RTT_LIBRARIES})
       set_target_properties(orocos-ocl-lua PROPERTIES COMPILE_FLAGS "-DOCL_COMPONENT_ONLY")
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
