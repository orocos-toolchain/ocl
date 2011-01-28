set( RTT_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )

orocos_plugin( rttlua-plugin lua_plugin.cpp ../rtt.cpp )
target_link_libraries(rttlua-plugin ${LUA_LIBRARIES})
