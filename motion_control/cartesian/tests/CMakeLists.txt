
DEPENDENT_OPTION( BUILD_CARTESIAN_TEST "Build Cartesian Components Test" ON "BUILD_CARTESIAN;BUILD_TESTS;BUILD_VIEWER" OFF )

IF ( BUILD_CARTESIAN_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( cartesian ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR}/hardware/axes ${PROJ_SOURCE_DIR}/hardware/comedi ${PROJ_SOURCE_DIR}/hardware/apci ${PROJ_SOURCE_DIR}/motion_control/naxes )
    TARGET_LINK_LIBRARIES( cartesian orocos-taskbrowser orocos-reporting orocos-viewer orocos-cartesian orocos-naxes orocos-kuka orocos-axes ${KDL_LIBS} ${KDLTK_LIBS} ACE)

    IF ( BUILD_COMEDI )
    TARGET_LINK_LIBRARIES( cartesian orocos-comedi )
    ENDIF ( BUILD_COMEDI )

    IF ( BUILD_APCI )
    TARGET_LINK_LIBRARIES( cartesian orocos-apci )
    ENDIF ( BUILD_APCI )
    
    TARGET_LINK_LIBRARIES( cartesian ${OROCOS_RTT_LIBS})


ENDIF ( BUILD_CARTESIAN_TEST )

