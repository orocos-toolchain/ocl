IF ( BUILD_TESTS)
OPTION( BUILD_LIAS_TEST "Build Lias Components Test" OFF )

IF ( BUILD_LIAS_TEST )

    SET (SRCS_CRS crs.cpp)
    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/axes/ )
    GLOBAL_ADD_TEST( crs ${SRCS_CRS} )
    TARGET_LINK_LIBRARIES( crs orocos-taskbrowser orocos-reporting orocos-lias orocos-axes ${OROCOS_RTT_LIBS})

    SET (SRCS_BASE base.cpp TestTcpTaskContext.cpp)
    GLOBAL_ADD_TEST( base ${SRCS_BASE} ) 
    TARGET_LINK_LIBRARIES( base orocos-taskbrowser orocos-reporting orocos-lias orocos-axes orocos-wrench kdltk kdl ACE ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_LIAS_TEST )

ENDIF ( BUILD_TESTS)
