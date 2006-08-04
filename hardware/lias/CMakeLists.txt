OPTION( BUILD_LIAS "Build LiAS Components" OFF )

IF ( BUILD_LIAS )
    
    SET( SRCS 
        BaseVelocityController.cpp       
        IP_Digital_24_DOutInterface.cpp  
        IP_FastDAC_AOutInterface.cpp   
        CRSnAxesVelocityController.cpp
        CombinedDigitalOutInterface.cpp  
        IP_Encoder_6_EncInterface.cpp    
        IP_OptoInput_DInInterface.cpp  
        liasclient.cpp
    )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-lias ${SRCS} )
    GLOBAL_ADD_INCLUDE( hardware/lias ${HPPS})

    ADD_SUBDIRECTORY( tests )
    ADD_SUBDIRECTORY( cpf )

ENDIF ( BUILD_LIAS )

