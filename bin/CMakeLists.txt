
# Local Deployer application
IF ( BUILD_TASKBROWSER AND BUILD_DEPLOYMENT)

    ADD_EXECUTABLE( deployer-${OROCOS_TARGET} deployer.cpp )
    PROGRAM_ADD_DEPS( deployer-${OROCOS_TARGET} orocos-taskbrowser orocos-deployment )
    TARGET_LINK_LIBRARIES( deployer-${OROCOS_TARGET} ${OROCOS_RTT_LIBS})
    INSTALL_TARGETS( /bin deployer-${OROCOS_TARGET} )

ENDIF ( BUILD_TASKBROWSER AND BUILD_DEPLOYMENT)

# Remote TaskBrowser application
IF ( BUILD_TASKBROWSER AND CORBA_ENABLED)

    ADD_EXECUTABLE( ctaskbrowser-${OROCOS_TARGET} ctaskbrowser.cpp )
    PROGRAM_ADD_DEPS( ctaskbrowser-${OROCOS_TARGET} orocos-taskbrowser )
    TARGET_LINK_LIBRARIES( ctaskbrowser-${OROCOS_TARGET} ${OROCOS_RTT_LIBS} orocos-rtt-corba-${OROCOS_TARGET} )
    INSTALL_TARGETS( /bin ctaskbrowser-${OROCOS_TARGET} )

ENDIF ( BUILD_TASKBROWSER AND CORBA_ENABLED)

# Deployment Server which can be accessed using ctaskbrowser
IF ( BUILD_DEPLOYMENT AND CORBA_ENABLED)

    ADD_EXECUTABLE( cdeployer-${OROCOS_TARGET} cdeployer.cpp )
    PROGRAM_ADD_DEPS( cdeployer-${OROCOS_TARGET} orocos-deployment )
    TARGET_LINK_LIBRARIES( cdeployer-${OROCOS_TARGET} ${OROCOS_RTT_LIBS} orocos-rtt-corba-${OROCOS_TARGET} )
    INSTALL_TARGETS( /bin cdeployer-${OROCOS_TARGET} )

ENDIF ( BUILD_DEPLOYMENT AND CORBA_ENABLED)

