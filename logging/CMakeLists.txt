OPTION( BUILD_LOGGING "Build realtime Logging service" ON)

if(BUILD_LOGGING AND NOT LOG4CPP_FOUND)
  message(SEND_ERROR "Can't build real-time logging service since log4cpp was not found.")
endif(BUILD_LOGGING AND NOT LOG4CPP_FOUND)

IF ( BUILD_LOGGING AND LOG4CPP_FOUND )

  FILE( GLOB HPPS [^.]*.hpp )

  set(LOG4CXXLIB_CPPS Log4cxxAppender.cpp)
  set(LOGLIB_CPPS Category.cpp LoggingEvent.cpp CategoryStream.cpp)
  set(LOGCOMP_CPPS Appender.cpp FileAppender.cpp OstreamAppender.cpp LoggingService.cpp)

  INCLUDE_DIRECTORIES( "${LOG4CPP_INCLUDE_DIRS}" )
  LINK_DIRECTORIES( "${LOG4CPP_LIBRARY_DIRS}" )

  orocos_component( orocos-ocl-logging ${LOGCOMP_CPPS} )
  orocos_library( orocos-ocl-log4cpp ${LOGLIB_CPPS} )

  orocos_install_headers( ${HPPS} INSTALL include/orocos/ocl )
  target_link_libraries( orocos-ocl-log4cpp ${LOG4CPP_LIBRARIES})
  target_link_libraries( orocos-ocl-logging ${LOG4CPP_LIBRARIES} orocos-ocl-log4cpp)

  if (LOG4CXX_FOUND)
    include_directories( "${LOG4CXX_INCLUDE_DIRS}" )
    orocos_component( orocos-ocl-log4cxx ${LOG4CXXLIB_CPPS} )
    target_link_libraries( orocos-ocl-log4cxx orocos-ocl-log4cpp ${LOG4CXX_LIBRARIES} )
  endif()

  orocos_generate_package( DEPENDS log4cpp )
  ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_LOGGING  AND LOG4CPP_FOUND )

