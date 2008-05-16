
# Local Deployer application
IF ( BUILD_TASKBROWSER AND BUILD_DEPLOYMENT)

    ADD_EXECUTABLE( deployer-${OROCOS_TARGET} deployer.cpp deployer-funcs.cpp )
    PROGRAM_ADD_DEPS( deployer-${OROCOS_TARGET} orocos-taskbrowser orocos-deployment )
    TARGET_LINK_LIBRARIES( deployer-${OROCOS_TARGET} ${OROCOS_RTT_LIBS} boost_program_options)
    INSTALL_TARGETS( /bin deployer-${OROCOS_TARGET} )

ENDIF ( BUILD_TASKBROWSER AND BUILD_DEPLOYMENT)

# Remote TaskBrowser application
IF ( BUILD_TASKBROWSER AND CORBA_ENABLED)

    ADD_EXECUTABLE( ctaskbrowser-${OROCOS_TARGET} ctaskbrowser.cpp )
    PROGRAM_ADD_DEPS( ctaskbrowser-${OROCOS_TARGET} orocos-taskbrowser )
    TARGET_LINK_LIBRARIES( ctaskbrowser-${OROCOS_TARGET} ${OROCOS_RTT_LIBS} ${OROCOS_RTT_CORBA_LIBS} )
    INSTALL_TARGETS( /bin ctaskbrowser-${OROCOS_TARGET} )

ENDIF ( BUILD_TASKBROWSER AND CORBA_ENABLED)

# Deployment Server which can be accessed using ctaskbrowser
IF ( BUILD_DEPLOYMENT AND CORBA_ENABLED)

    ADD_EXECUTABLE( cdeployer-${OROCOS_TARGET} cdeployer.cpp deployer-funcs.cpp )
    PROGRAM_ADD_DEPS( cdeployer-${OROCOS_TARGET} orocos-deployment-corba )
    TARGET_LINK_LIBRARIES( cdeployer-${OROCOS_TARGET} ${OROCOS_RTT_LIBS} ${OROCOS_RTT_CORBA_LIBS} boost_program_options )
    INSTALL_TARGETS( /bin cdeployer-${OROCOS_TARGET} )

    ADD_EXECUTABLE( deployer-corba-${OROCOS_TARGET} deployer-corba.cpp deployer-funcs.cpp )
    PROGRAM_ADD_DEPS( deployer-corba-${OROCOS_TARGET} orocos-deployment-corba orocos-taskbrowser )
    TARGET_LINK_LIBRARIES( deployer-corba-${OROCOS_TARGET} ${OROCOS_RTT_LIBS} ${OROCOS_RTT_CORBA_LIBS} boost_program_options )
    INSTALL_TARGETS( /bin deployer-corba-${OROCOS_TARGET} )

ENDIF ( BUILD_DEPLOYMENT AND CORBA_ENABLED)

