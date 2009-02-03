
DEPENDENT_OPTION( BUILD_NAXES_TEST "Build NAxes Components Test" ON "BUILD_NAXES;BUILD_TESTS" OFF)

IF ( BUILD_NAXES_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/axes/ ${PROJECT_SOURCE_DIR}/hardware/comedi/ ${PROJECT_SOURCE_DIR}/hardware/apci/ )
    GLOBAL_ADD_TEST( naxes ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    PROGRAM_ADD_DEPS( naxes orocos-taskbrowser orocos-reporting orocos-naxes orocos-kuka orocos-axes )
    TARGET_LINK_LIBRARIES( naxes orocos-kdl )

    IF ( BUILD_COMEDI )
    PROGRAM_ADD_DEPS( naxes orocos-comedi )
    ENDIF ( BUILD_COMEDI )

    IF ( BUILD_APCI )
    PROGRAM_ADD_DEPS( naxes orocos-apci )
    ENDIF ( BUILD_APCI )
    
    TARGET_LINK_LIBRARIES( naxes ${OROCOS_RTT_LIBS})

ENDIF ( BUILD_NAXES_TEST )

