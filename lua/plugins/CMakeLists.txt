CMAKE_DEPENDENT_OPTION( LUA_PLUGIN "Build lua scripting plugin" ON "BUILD_LUA_COMPONENT" OFF )

IF ( LUA_PLUGIN )
  SET( RTT_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
  # ADD_RTT_PLUGIN( rttlua ${RTT_VERSION} lua_plugin.cpp )
  add_library(rttlua-${OROCOS_TARGET}_plugin SHARED  lua_plugin.cpp)
  target_link_libraries(rttlua-${OROCOS_TARGET}_plugin)
ENDIF ( LUA_PLUGIN )