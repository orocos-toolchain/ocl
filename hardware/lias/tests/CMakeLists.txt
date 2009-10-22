DEPENDENT_OPTION( BUILD_LIAS_TEST "Build Lias Components Test" OFF "BUILD_LIAS;BUILD_TESTS" OFF)

IF ( BUILD_LIAS_TEST )

    SET (SRCS_CRS crs.cpp)
    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/axes/ )
    GLOBAL_ADD_TEST( crs ${SRCS_CRS} )
    PROGRAM_ADD_DEPS( crs orocos-taskbrowser orocos-reporting orocos-lias orocos-axes  orocos-kdltk)
    TARGET_LINK_LIBRARIES( crs orocos-kdl ACE ${OROCOS_RTT_LIBS})

    SET (SRCS_BASE base.cpp TestTcpTaskContext.cpp)
    GLOBAL_ADD_TEST( base ${SRCS_BASE} )
    PROGRAM_ADD_DEPS( base orocos-taskbrowser orocos-reporting orocos-lias orocos-axes orocos-wrench  orocos-kdltk)
    TARGET_LINK_LIBRARIES( base orocos-kdl ACE ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_LIAS_TEST )

