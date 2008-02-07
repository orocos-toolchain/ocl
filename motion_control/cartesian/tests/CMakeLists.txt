
DEPENDENT_OPTION( BUILD_CARTESIAN_TEST "Build Cartesian Components Test" ON "BUILD_CARTESIAN;BUILD_TESTS" OFF )

IF ( BUILD_CARTESIAN_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( cartesian ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR}/hardware/axes ${PROJ_SOURCE_DIR}/hardware/comedi ${PROJ_SOURCE_DIR}/hardware/apci ${PROJ_SOURCE_DIR}/motion_control/naxes )
    program_add_deps( cartesian orocos-taskbrowser orocos-reporting orocos-cartesian orocos-kuka)

    IF ( BUILD_COMEDI )
    program_add_deps( cartesian orocos-comedi )
    ENDIF ( BUILD_COMEDI )

    IF ( BUILD_APCI )
    program_add_deps( cartesian orocos-apci )
    ENDIF ( BUILD_APCI )
    
    TARGET_LINK_LIBRARIES( cartesian ${OROCOS_RTT_LIBS} ${KDL_LIBS} ${KDLTK_LIBS} ${KDLTKCORBA_LIBS} orocos-rtt-${FLAVOR_TARGET})


ENDIF ( BUILD_CARTESIAN_TEST )

