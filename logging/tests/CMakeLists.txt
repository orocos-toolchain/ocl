
DEPENDENT_OPTION( BUILD_LOGGING_TEST "Build logging tests" ON "BUILD_LOGGING;BUILD_TESTS" OFF)

IF ( BUILD_LOGGING_TEST )

  INCLUDE_DIRECTORIES( "${CMAKE_SOURCE_DIR}/rtalloc" )

  INCLUDE_DIRECTORIES( ${LOG4CPP_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${LOG4CPP_LIBRARY_DIRS} )

  CREATE_COMPONENT(orocos-logging-tests 
	TestComponent.cpp
	TestLoggingAvailability.cpp
	)
  COMPONENT_ADD_DEPS(orocos-logging-tests orocos-logging)
  COMPONENT_ADD_LIBS(orocos-logging-tests log4cpp)

  GLOBAL_ADD_TEST(testlogging testlogging.cpp)
  PROGRAM_ADD_DEPS(testlogging orocos-logging)

  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink 
	${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)

ENDIF ( BUILD_LOGGING_TEST )

