
IF ( BUILD_TASKBROWSER AND BUILD_DEPLOYMENT)

    ADD_EXECUTABLE( deployer deployer.cpp )
    TARGET_LINK_LIBRARIES( deployer orocos-taskbrowser orocos-deployment ${OROCOS_RTT_LIBS})
    INSTALL_TARGETS( /bin deployer )

ENDIF ( BUILD_TASKBROWSER AND BUILD_DEPLOYMENT)

IF ( BUILD_TASKBROWSER AND CORBA_ENABLED)

    ADD_EXECUTABLE( ctaskbrowser ctaskbrowser.cpp )
    TARGET_LINK_LIBRARIES( ctaskbrowser orocos-taskbrowser ${OROCOS_RTT_LIBS})
    INSTALL_TARGETS( /bin ctaskbrowser )

ENDIF ( BUILD_TASKBROWSER AND CORBA_ENABLED)

