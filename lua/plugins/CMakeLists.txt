set( RTT_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
add_library(rttlua-${OROCOS_TARGET}_plugin SHARED  lua_plugin.cpp ../rtt.cpp)
set_target_properties(rttlua-${OROCOS_TARGET}_plugin PROPERTIES 
  DEFINE_SYMBOL "OCL_DLL_EXPORT"
  VERSION ${OCL_VERSION}
  SOVERSION ${OCL_VERSION_MAJOR}.${OCL_VERSION_MINOR}
  INSTALL_RPATH_USE_LINK_PATH 1
  )

target_link_libraries(rttlua-${OROCOS_TARGET}_plugin)
install(TARGETS rttlua-${OROCOS_TARGET}_plugin LIBRARY DESTINATION lib/orocos/plugins)
