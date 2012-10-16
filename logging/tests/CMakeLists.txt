
CMAKE_DEPENDENT_OPTION( BUILD_LOGGING_TEST "Build logging tests" ON "BUILD_LOGGING;BUILD_TESTS" OFF)

IF ( BUILD_LOGGING_TEST )

  INCLUDE_DIRECTORIES( ${LOG4CPP_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${LOG4CPP_LIBRARY_DIRS} )

  orocos_component(orocos-logging-tests
	TestComponent.cpp
	TestLoggingAvailability.cpp
	)
  target_link_libraries(orocos-logging-tests orocos-ocl-logging log4cpp)

  GLOBAL_ADD_TEST(testlogging testlogging.cpp)
  target_link_libraries(testlogging orocos-ocl-logging)

  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink 
	${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)

ENDIF ( BUILD_LOGGING_TEST )

