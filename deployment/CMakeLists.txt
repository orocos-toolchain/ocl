OPTION( BUILD_DEPLOYMENT "Build Deployment Component" ON )

IF ( BUILD_DEPLOYMENT )

  find_package(RTTPlugin REQUIRED rtt-marshalling rtt-scripting)
  
  find_package(Boost 1.36 REQUIRED filesystem system)

    # This gathers all the .cpp files into the variable 'SRCS'
    SET( HPPS DeploymentComponent.hpp )
    SET( SRCS DeploymentComponent.cpp ComponentLoader.cpp ${CMAKE_CURRENT_BINARY_DIR}/comppath.cpp)

    orocos_library( orocos-ocl-deployment ${SRCS})
    SET_TARGET_PROPERTIES( orocos-ocl-deployment PROPERTIES
      DEFINE_SYMBOL OCL_DLL_EXPORT)
    orocos_install_headers( ${HPPS} INSTALL include/orocos/ocl )
    
    IF(NOT WIN32)
      TARGET_LINK_LIBRARIES( orocos-ocl-deployment "dl" )
    ENDIF()
    TARGET_LINK_LIBRARIES( orocos-ocl-deployment ${RTT_PLUGIN_rtt-marshalling_${OROCOS_TARGET}_LIBRARIES} ${RTT_PLUGIN_rtt-scripting_${OROCOS_TARGET}_LIBRARIES})
    TARGET_LINK_LIBRARIES( orocos-ocl-deployment ${Boost_FILESYSTEM_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})
    
    IF (OROCOS-RTT_CORBA_FOUND)
        orocos_library(orocos-ocl-deployment-corba CorbaDeploymentComponent.cpp )
        SET_TARGET_PROPERTIES( orocos-ocl-deployment-corba PROPERTIES
          DEFINE_SYMBOL OCL_DLL_EXPORT)
        orocos_install_headers( CorbaDeploymentComponent.hpp INSTALL include/orocos/ocl )
        TARGET_LINK_LIBRARIES( orocos-ocl-deployment-corba orocos-ocl-deployment)
        TARGET_LINK_LIBRARIES( orocos-ocl-deployment-corba ${OROCOS-RTT_CORBA_LIBRARIES})
    ENDIF(OROCOS-RTT_CORBA_FOUND)
    
    # rospack support
    IF ( ROS_ROOT )
        # Catkin style of fetching ROS deps
        FIND_PACKAGE( ROS COMPONENTS roslib )
        INCLUDE_DIRECTORIES( ${ROS_INCLUDE_DIRS} )
        TARGET_LINK_LIBRARIES( orocos-ocl-deployment ${ROS_LIBRARIES})
        ADD_DEFINITIONS( -DHAS_ROSLIB )
        # TODO: The following block to get roslib is deprecated as of ROS Fuerte, but is left in for pre-Fuerte compatibility
        IF(NOT ROS_FOUND)
          set(roslib_PACKAGE_PATH ${ROS_ROOT}/core/roslib) ### XXX hardcoded
          include_directories( ${roslib_PACKAGE_PATH}/include )
          find_library(ROS_LIB roslib ${roslib_PACKAGE_PATH}/lib )
          TARGET_LINK_LIBRARIES( orocos-ocl-deployment ${ROS_LIB} )
        ENDIF(not ROS_FOUND)
	# Necessary when building Debian packages:
	set(ROS_STACK_DIR_FINAL $ENV{ROS_STACK_DIR_FINAL})
	if (ROS_STACK_DIR_FINAL)
	  set(DEFAULT_COMPONENT_PATH "${ROS_STACK_DIR_FINAL}/orocos_toolchain/install/lib/orocos")
	else(ROS_STACK_DIR_FINAL)
          set(DEFAULT_COMPONENT_PATH "${PROJECT_SOURCE_DIR}/../install/lib/orocos" CACHE STRING "Default component lookup path when none given. (semi-)colon separated list." FORCE)
	endif(ROS_STACK_DIR_FINAL)
    ELSE( ROS_ROOT)
	SET(DEFAULT_COMPONENT_PATH "${OROCOS-RTT_PATH}/lib/orocos" CACHE STRING "Default component lookup path when none given. (semi-)colon separated list." )
    ENDIF( ROS_ROOT )

    configure_file(comppath.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/comppath.cpp @ONLY)

    if (NOT (${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR}))
        # This to reduce upgrade pains for in-tree upgraders:
        execute_process( COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_SOURCE_DIR}/comppath.cpp )
    endif()

    orocos_generate_package()

    ADD_SUBDIRECTORY( tests )

    # This requires the orocos-rtt library to be carefully tagged
    # with the necessary 'export' attributes, or unresolved symbols
    # will appear. Especially template classes and exceptions are 
    # sensitive.
    # SET_SOURCE_FILES_PROPERTIES( ${SRCS} PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")

ENDIF ( BUILD_DEPLOYMENT )
