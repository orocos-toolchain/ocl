SET( RTT_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
add_library(rttlua-${OROCOS_TARGET}_plugin SHARED  lua_plugin.cpp)
target_link_libraries(rttlua-${OROCOS_TARGET}_plugin)
