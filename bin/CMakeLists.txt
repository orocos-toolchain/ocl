
# Local Deployer application
IF ( BUILD_TASKBROWSER AND BUILD_DEPLOYMENT)

    ADD_EXECUTABLE( deployer deployer.cpp )
    TARGET_LINK_LIBRARIES( deployer orocos-taskbrowser orocos-deployment ${OROCOS_RTT_LIBS})
    INSTALL_TARGETS( /bin deployer )

ENDIF ( BUILD_TASKBROWSER AND BUILD_DEPLOYMENT)

# Remote TaskBrowser application
IF ( BUILD_TASKBROWSER AND CORBA_ENABLED)

    ADD_EXECUTABLE( ctaskbrowser ctaskbrowser.cpp )
    TARGET_LINK_LIBRARIES( ctaskbrowser orocos-taskbrowser ${OROCOS_RTT_LIBS} orocos-rtt-corba-${OROCOS_TARGET} )
    INSTALL_TARGETS( /bin ctaskbrowser )

ENDIF ( BUILD_TASKBROWSER AND CORBA_ENABLED)

# Deployment Server which can be accessed using ctaskbrowser
IF ( BUILD_DEPLOYMENT AND CORBA_ENABLED)

    ADD_EXECUTABLE( cdeployer cdeployer.cpp )
    TARGET_LINK_LIBRARIES( cdeployer orocos-deployment ${OROCOS_RTT_LIBS} orocos-rtt-corba-${OROCOS_TARGET} )
    INSTALL_TARGETS( /bin cdeployer )

ENDIF ( BUILD_DEPLOYMENT AND CORBA_ENABLED)

