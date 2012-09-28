OPTION( BUILD_DEPLOYMENT "Build Deployment Component" ON )

IF ( BUILD_DEPLOYMENT )

  find_package(RTTPlugin REQUIRED rtt-marshalling rtt-scripting)
  
  find_package(Boost 1.36 REQUIRED filesystem system)

    # This gathers all the .cpp files into the variable 'SRCS'
    SET( HPPS DeploymentComponent.hpp )
    SET( SRCS DeploymentComponent.cpp )

    orocos_library( orocos-ocl-deployment ${SRCS})
    SET_TARGET_PROPERTIES( orocos-ocl-deployment PROPERTIES
      DEFINE_SYMBOL OCL_DLL_EXPORT)
    orocos_install_headers( ${HPPS} INSTALL include/orocos/ocl )
    
    TARGET_LINK_LIBRARIES( orocos-ocl-deployment ${RTT_PLUGIN_rtt-marshalling_${OROCOS_TARGET}_LIBRARIES} ${RTT_PLUGIN_rtt-scripting_${OROCOS_TARGET}_LIBRARIES})
    
    IF (OROCOS-RTT_CORBA_FOUND)
        orocos_library(orocos-ocl-deployment-corba CorbaDeploymentComponent.cpp )
        SET_TARGET_PROPERTIES( orocos-ocl-deployment-corba PROPERTIES
          DEFINE_SYMBOL OCL_DLL_EXPORT)
        orocos_install_headers( CorbaDeploymentComponent.hpp INSTALL include/orocos/ocl )
        TARGET_LINK_LIBRARIES( orocos-ocl-deployment-corba orocos-ocl-deployment)
        TARGET_LINK_LIBRARIES( orocos-ocl-deployment-corba ${OROCOS-RTT_CORBA_LIBRARIES})
    ENDIF(OROCOS-RTT_CORBA_FOUND)
    
    orocos_generate_package()

    ADD_SUBDIRECTORY( tests )

    # This requires the orocos-rtt library to be carefully tagged
    # with the necessary 'export' attributes, or unresolved symbols
    # will appear. Especially template classes and exceptions are 
    # sensitive.
    # SET_SOURCE_FILES_PROPERTIES( ${SRCS} PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")

ENDIF ( BUILD_DEPLOYMENT )
