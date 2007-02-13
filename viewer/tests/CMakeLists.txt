DEPENDENT_OPTION( BUILD_VIEWER_TEST "Build Viewer Test" ON "BUILD_VIEWER;BUILD_TESTS;BUILD_KUKA" OFF )

IF ( BUILD_VIEWER_TEST )

    SET (SRCS main.cpp)

    GLOBAL_ADD_TEST( viewer ${SRCS} )

    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR}/hardware/axes ${PROJ_SOURCE_DIR}/hardware/comedi ${PROJ_SOURCE_DIR}/hardware/apci )
    TARGET_LINK_LIBRARIES( viewer orocos-taskbrowser orocos-reporting orocos-kuka orocos-axes orocos-viewer ACE )

    IF ( BUILD_COMEDI )
    TARGET_LINK_LIBRARIES( viewer orocos-comedi )
    ENDIF ( BUILD_COMEDI )

    IF ( BUILD_APCI )
    TARGET_LINK_LIBRARIES( viewer orocos-apci )
    ENDIF ( BUILD_APCI )
    
    TARGET_LINK_LIBRARIES( viewer ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_VIEWER_TEST )
