DEPENDENT_OPTION( BUILD_LIAS "Build LiAS Components" OFF "BUILD_HARDWARE" OFF)

#Always process subdirs
ADD_SUBDIRECTORY( tests )

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

    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/hardware/axes/ )
    GLOBAL_ADD_COMPONENT( orocos-lias ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

ENDIF ( BUILD_LIAS )

