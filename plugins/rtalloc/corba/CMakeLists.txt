IF (ENABLE_CORBA AND BUILD_RTALLOC)

  SET( IDLS RTallocTypes.idl )
  SET( CPPS RTallocCorbaToolkit.cpp )
  SET( HPPS RTallocCorbaToolkit.hpp )

#  ORO_ADD_CORBA_SERVERS(CPPS HPPS ${IDLS} )

  GLOBAL_ADD_INCLUDE( ocl ${HPPS})

  #This has been added in order to generate files before make -j<N>
  # make -j<N> causes corruption of generated files because of overwriting
  ADD_CUSTOM_TARGET( plugins-rtalloc-idl DEPENDS ${CPPS} ${HPPS} COMMENT "Generating sources from .idl files")

  INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/. )
  INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR}/. )

  ADD_LIBRARY(orocos-rtalloctk-corba-${OROCOS_TARGET} SHARED ${CPPS})
  TARGET_LINK_LIBRARIES(orocos-rtalloctk-corba-${OROCOS_TARGET} 
	orocos-rtalloctk-${OROCOS_TARGET} 
	orocos-rtt-corba-${OROCOS_TARGET}
	${CORBA_LIBRARIES}
	) 
  SET_TARGET_PROPERTIES( orocos-rtalloctk-corba-${OROCOS_TARGET} PROPERTIES
	OUTPUT_NAME orocos-rtalloctk-corba-${OROCOS_TARGET}
	CLEAN_DIRECT_OUTPUT 1
	COMPILE_FLAGS "${CMAKE_CXX_FLAGS_ADD}"
	SOVERSION "${OCL_VERSION_MAJOR}.${OCL_VERSION_MINOR}"
	VERSION "${OCL_VERSION}")
  INSTALL(TARGETS orocos-rtalloctk-corba-${OROCOS_TARGET} LIBRARY DESTINATION lib RUNTIME DESTINATION bin )

  ADD_SUBDIRECTORY( tests )

ENDIF (ENABLE_CORBA AND BUILD_RTALLOC)
