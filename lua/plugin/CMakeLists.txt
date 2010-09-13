macro(ADD_RTT_PLUGIN name version)
  ADD_LIBRARY(${name}-${OROCOS_TARGET}_plugin SHARED ${ARGN})
  SET_TARGET_PROPERTIES( ${name}-${OROCOS_TARGET}_plugin PROPERTIES
    VERSION "${version}"
    OUTPUT_NAME ${name}-${OROCOS_TARGET}
    COMPILE_DEFINITIONS "${RTT_DEFINITIONS}"
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
    CLEAN_DIRECT_OUTPUT 1)

  if (DLL_EXPORT_PREFIX)
    string(TOUPPER ${DLL_EXPORT_PREFIX} UDLL_EXPORT_PREFIX )
    set(DEFINE_PREFIX "RTT_${UDLL_EXPORT_PREFIX}_DLL_EXPORT")
    SET_TARGET_PROPERTIES( ${name}-${OROCOS_TARGET}_plugin PROPERTIES DEFINE_SYMBOL "${DEFINE_PREFIX}")
  endif()

  target_link_libraries(${name}-${OROCOS_TARGET}_plugin orocos-rtt-${OROCOS_TARGET}_dynamic)
  
  if(DLL_EXPORT_PREFIX)
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/rtt-${DLL_EXPORT_PREFIX}-config.h.in ${CMAKE_CURRENT_BINARY_DIR}/rtt-${DLL_EXPORT_PREFIX}-config.h @ONLY)
  endif()

  # Note: plugins do get the symlinks
  install(TARGETS ${name}-${OROCOS_TARGET}_plugin
          ARCHIVE DESTINATION lib/orocos/plugins
          LIBRARY DESTINATION lib/orocos/plugins)

  get_target_property(PLUGINLIB_DIR ${name}-${OROCOS_TARGET}_plugin LOCATION)

  if (PROJ_BINARY_DIR)
    add_custom_command(TARGET ${name}-${OROCOS_TARGET}_plugin POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJ_BINARY_DIR}/rtt/plugins
          COMMAND ${CMAKE_COMMAND} -E copy ${PLUGINLIB_DIR} ${PROJ_BINARY_DIR}/rtt/plugins)
  if (WIN32)
    get_target_property(DEBUG_PLUGINLIB_DIR ${name}-${OROCOS_TARGET}_plugin DEBUG_LOCATION)
    add_custom_command(TARGET ${name}-${OROCOS_TARGET}_plugin POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${DEBUG_PLUGINLIB_DIR} ${PROJ_BINARY_DIR}/rtt/types)
  endif()
  endif(PROJ_BINARY_DIR)
  
endmacro(ADD_RTT_PLUGIN name)

CMAKE_DEPENDENT_OPTION( LUA_PLUGIN "Build lua scripting plugin" ON "BUILD_LUA_COMPONENT" OFF )

IF ( LUA_PLUGIN )
  SET( RTT_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}" )
  ADD_RTT_PLUGIN( rttlua ${RTT_VERSION} lua_plugin.cpp )
ENDIF ( LUA_PLUGIN )