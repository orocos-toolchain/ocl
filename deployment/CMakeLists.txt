OPTION( BUILD_DEPLOYMENT "Build Deployment Component" ON )

IF ( BUILD_DEPLOYMENT )

  find_package(RTTPlugin REQUIRED rtt-marshalling rtt-scripting)
  
  find_package(Boost 1.36 REQUIRED filesystem system)

    # This gathers all the .cpp files into the variable 'SRCS'
    SET( HPPS DeploymentComponent.hpp )
    SET( SRCS DeploymentComponent.cpp ComponentLoader.cpp ${CMAKE_CURRENT_BINARY_DIR}/comppath.cpp)

    orocos_library( orocos-deployment ${SRCS})
    orocos_install_headers( ${HPPS} INSTALL include/orocos/deployment )

    TARGET_LINK_LIBRARIES( orocos-deployment "dl" )
    TARGET_LINK_LIBRARIES( orocos-deployment ${RTT_PLUGIN_rtt-marshalling_LIBRARIES} ${RTT_PLUGIN_rtt-scripting_LIBRARIES})
    TARGET_LINK_LIBRARIES( orocos-deployment ${Boost_FILESYSTEM_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})
    target_link_libraries( orocos-deployment LINK_INTERFACE_LIBRARIES )
    
    IF (OROCOS-RTT_CORBA_FOUND)
        orocos_library(orocos-deployment-corba CorbaDeploymentComponent.cpp )
        GLOBAL_ADD_INCLUDE( ocl CorbaDeploymentComponent.hpp)
        TARGET_LINK_LIBRARIES( orocos-deployment-corba orocos-deployment)
	    TARGET_LINK_LIBRARIES( orocos-deployment-corba ${OROCOS-RTT_CORBA_LIBRARIES})
    ENDIF(OROCOS-RTT_CORBA_FOUND)
    
    # rospack support
    IF ( ROS_ROOT )
        rosbuild_find_ros_package( roslib )
        include_directories( ${roslib_PACKAGE_PATH}/include )
        find_library(ROS_LIB roslib ${roslib_PACKAGE_PATH}/lib )
        TARGET_LINK_LIBRARIES( orocos-deployment ${ROS_LIB} )
        ADD_DEFINITIONS( -DHAS_ROSLIB )
        SET(DEFAULT_COMPONENT_PATH "${PROJECT_SOURCE_DIR}/lib/orocos" CACHE STRING "Default component lookup path when none given. (semi-)colon separated list." FORCE)
    ELSE( ROS_ROOT)
	SET(DEFAULT_COMPONENT_PATH "${OROCOS-RTT_PATH}/lib/orocos" CACHE STRING "Default component lookup path when none given. (semi-)colon separated list." )
    ENDIF( ROS_ROOT )

    configure_file(comppath.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/comppath.cpp @ONLY)

    ADD_SUBDIRECTORY( tests )

    # This requires the orocos-rtt library to be carefully tagged
    # with the necessary 'export' attributes, or unresolved symbols
    # will appear. Especially template classes and exceptions are 
    # sensitive.
    # SET_SOURCE_FILES_PROPERTIES( ${SRCS} PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")

ENDIF ( BUILD_DEPLOYMENT )
