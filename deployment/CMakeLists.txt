OPTION( BUILD_DEPLOYMENT "Build Deployment Component" ON )

IF ( BUILD_DEPLOYMENT )

  find_package(RTTPlugin REQUIRED rtt-marshalling rtt-scripting)
  
  find_package(Boost 1.36 REQUIRED filesystem system)

  # Better inform user instead of hiding the option.
  IF(NOT OROCOS_RTT_1.2 )
    MESSAGE(FATAL_ERROR "Deployment Component requires Orocos RTT 1.1.0 or newer." )
  ENDIF(NOT OROCOS_RTT_1.2 )

    # This gathers all the .cpp files into the variable 'SRCS'
    SET( HPPS DeploymentComponent.hpp )
    SET( SRCS DeploymentComponent.cpp ComponentLoader.cpp comppath.cpp)

    GLOBAL_ADD_COMPONENT( orocos-deployment ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

    COMPONENT_ADD_LIBS( orocos-deployment "dl" )
    COMPONENT_ADD_LIBS( orocos-deployment ${RTT_PLUGIN_rtt-marshalling_LIBRARIES} ${RTT_PLUGIN_rtt-scripting_LIBRARIES})
    COMPONENT_ADD_LIBS( orocos-deployment ${Boost_FILESYSTEM_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})
    
    IF (CORBA_ENABLED)
        GLOBAL_ADD_COMPONENT(orocos-deployment-corba CorbaDeploymentComponent.cpp)
        GLOBAL_ADD_INCLUDE( ocl CorbaDeploymentComponent.hpp)
        COMPONENT_ADD_DEPS( orocos-deployment-corba orocos-deployment)
	COMPONENT_ADD_LIBS( orocos-deployment-corba ${OROCOS_RTT_CORBA_LIBS})
    ENDIF(CORBA_ENABLED)
     
    ADD_SUBDIRECTORY( tests )

    SET(DEFAULT_COMPONENT_PATH "${OROCOS_INSTALL}/lib/orocos" CACHE STRING "Default component lookup path when none given.")
    configure_file(comppath.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/comppath.cpp @ONLY)

    # This requires the orocos-rtt library to be carefully tagged
    # with the necessary 'export' attributes, or unresolved symbols
    # will appear. Especially template classes and exceptions are 
    # sensitive.
    # SET_SOURCE_FILES_PROPERTIES( ${SRCS} PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")

ENDIF ( BUILD_DEPLOYMENT )
