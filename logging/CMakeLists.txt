OPTION( BUILD_LOGGING "Build realtime Logging service" ON)

IF ( BUILD_LOGGING )
  if(NOT LOG4CPP_FOUND)
    message(SEND_ERROR "Can't build real-time logging service since log4cpp was not found.")
  endif()

  FILE( GLOB HPPS [^.]*.hpp )

  set(LOGLIB_CPPS Category.cpp LoggingEvent.cpp)
  set(LOGCOMP_CPPS Appender.cpp FileAppender.cpp OstreamAppender.cpp LoggingService.cpp)

  INCLUDE_DIRECTORIES( "${LOG4CPP_INCLUDE_DIRS}" )
  LINK_DIRECTORIES( "${LOG4CPP_LIBRARY_DIRS}" )

  orocos_component( orocos-logging ${LOGCOMP_CPPS} INSTALL lib/orocos)
  orocos_library( orocos-log4cpp ${LOGLIB_CPPS} )

  GLOBAL_ADD_INCLUDE( ocl ${HPPS})
  target_link_libraries( orocos-log4cpp "${LOG4CPP_LIBRARIES}")
  target_link_libraries( orocos-logging "${LOG4CPP_LIBRARIES}" orocos-log4cpp)

  ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_LOGGING )

