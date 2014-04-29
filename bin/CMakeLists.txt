SET(EXTRA_LIBRARIES)
SET(EXTRA_DEPS)

IF ( BUILD_RTALLOC )
  # default to 512 kilobytes - see minimum size warning in deployer-funcs.cpp
  SET(ORO_DEFAULT_RTALLOC_SIZE 524288 CACHE STRING "Real-time memory pool size (bytes) for operations and logging.")
  ADD_DEFINITIONS("-DORO_BUILD_RTALLOC -DORO_DEFAULT_RTALLOC_SIZE=${ORO_DEFAULT_RTALLOC_SIZE}")
ELSE ( BUILD_RTALLOC )
  ADD_DEFINITIONS("-DORO_DEFAULT_RTALLOC_SIZE=0")
ENDIF ( BUILD_RTALLOC )

# TODO get this from RTT config
IF (UNIX AND NOT APPLE AND NOT SSC_NO_CPU_AFFINITY)
  ADD_DEFINITIONS("-DORO_SUPPORT_CPU_AFFINITY=1")
ENDIF (UNIX AND NOT APPLE AND NOT SSC_NO_CPU_AFFINITY)

IF ( BUILD_LOGGING )
  INCLUDE_DIRECTORIES( "${LOG4CPP_INCLUDE_DIRS}" )
  LINK_DIRECTORIES( "${LOG4CPP_LIBRARY_DIRS}" )
  LIST(APPEND EXTRA_LIBRARIES "${LOG4CPP_LIBRARY}")
  LIST(APPEND EXTRA_DEPS "orocos-ocl-log4cpp")
  ADD_DEFINITIONS("-DORO_BUILD_LOGGING")
ENDIF ( BUILD_LOGGING )

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(installpath.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/installpath.hpp @ONLY)

# Local Deployer application
IF ( BUILD_TASKBROWSER AND BUILD_DEPLOYMENT)

    orocos_executable( deployer deployer.cpp deployer-funcs.cpp )
    program_add_deps( deployer ${EXTRA_DEPS} )
    program_add_deps( deployer ${OROCOS-RTT_LIBRARIES} orocos-ocl-taskbrowser orocos-ocl-deployment ${Boost_PROGRAM_OPTIONS_LIBRARY} ${EXTRA_LIBRARIES})
    list(APPEND UTILITY_SCRIPTS deployer)

ENDIF ( BUILD_TASKBROWSER AND BUILD_DEPLOYMENT)
IF ( BUILD_DEPLOYMENT)

    orocos_executable( rttscript deployer.cpp deployer-funcs.cpp )
    program_add_deps( rttscript ${EXTRA_DEPS} )
    program_add_deps( rttscript ${OROCOS-RTT_LIBRARIES} orocos-ocl-deployment ${Boost_PROGRAM_OPTIONS_LIBRARY} ${EXTRA_LIBRARIES})
    set_target_properties( rttscript PROPERTIES COMPILE_DEFINITIONS RTT_SCRIPT_PROGRAM )
    list(APPEND UTILITY_SCRIPTS rttscript)

ENDIF ( BUILD_DEPLOYMENT)

# Remote TaskBrowser application
IF ( BUILD_TASKBROWSER AND OROCOS-RTT_CORBA_FOUND)

    orocos_executable( ctaskbrowser ctaskbrowser.cpp )
    program_add_deps( ctaskbrowser ${EXTRA_DEPS} )
    program_add_deps( ctaskbrowser ${OROCOS-RTT_LIBRARIES} orocos-ocl-taskbrowser ${OROCOS-RTT_CORBA_LIBRARIES} ${EXTRA_LIBRARIES} )
    list(APPEND UTILITY_SCRIPTS ctaskbrowser)

ENDIF ( BUILD_TASKBROWSER AND OROCOS-RTT_CORBA_FOUND)

# Deployment Server which can be accessed using ctaskbrowser
IF ( BUILD_DEPLOYMENT AND OROCOS-RTT_CORBA_FOUND)

    orocos_executable( cdeployer cdeployer.cpp deployer-funcs.cpp )
    program_add_deps( cdeployer ${EXTRA_DEPS} )
    program_add_deps( cdeployer ${OROCOS-RTT_LIBRARIES} ${OROCOS-RTT_CORBA_LIBRARIES} orocos-ocl-deployment-corba ${Boost_PROGRAM_OPTIONS_LIBRARY} ${EXTRA_LIBRARIES})
    list(APPEND UTILITY_SCRIPTS cdeployer)

    orocos_executable( deployer-corba deployer-corba.cpp deployer-funcs.cpp )
    program_add_deps( deployer-corba ${EXTRA_DEPS} )
    program_add_deps( deployer-corba ${OROCOS-RTT_LIBRARIES} ${OROCOS-RTT_CORBA_LIBRARIES} orocos-ocl-deployment-corba orocos-ocl-taskbrowser ${Boost_PROGRAM_OPTIONS_LIBRARY} ${EXTRA_LIBRARIES})
    list(APPEND UTILITY_SCRIPTS deployer-corba)

ENDIF ( BUILD_DEPLOYMENT AND OROCOS-RTT_CORBA_FOUND)

# Utility scripts
install(PROGRAMS ${UTILITY_SCRIPTS} DESTINATION bin)
