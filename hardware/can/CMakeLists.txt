DEPENDENT_OPTION( BUILD_CAN "Build Can Component" OFF "BUILD_HARDWARE" OFF)
DEPENDENT_OPTION( BUILD_CANPIE "Build Canpie Controller (LXRT only)" OFF "BUILD_CAN;OS_LXRT" OFF)
DEPENDENT_OPTION( BUILD_PCAN "Build Peak Systems CAN Controller (soft realtime only)" OFF "BUILD_CAN" OFF)
DEPENDENT_OPTION( BUILD_SOCKET_CAN "Build socket CAN controller" OFF "BUILD_CAN" OFF)
DEPENDENT_OPTION( BUILD_RTSOCKET_CAN "Build real-time socket CAN controller" OFF "BUILD_CAN;OS_XENOMAI" OFF)

IF ( BUILD_CAN )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB DEVSRCS can/[^.]*.cpp )
    FILE( GLOB DEVHPPS can/[^.]*.hpp can/[^.]*.h )

    LIST( REMOVE_ITEM DEVSRCS ${CMAKE_CURRENT_SOURCE_DIR}/can/CANPieController.cpp)
    LIST( REMOVE_ITEM DEVHPPS ${CMAKE_CURRENT_SOURCE_DIR}/can/CANPieController.hpp)
    LIST( REMOVE_ITEM DEVSRCS ${CMAKE_CURRENT_SOURCE_DIR}/can/PCANController.cpp)
    LIST( REMOVE_ITEM DEVHPPS ${CMAKE_CURRENT_SOURCE_DIR}/can/PCANController.hpp)
    LIST( REMOVE_ITEM DEVSRCS ${CMAKE_CURRENT_SOURCE_DIR}/can/SocketCANController.cpp)
    LIST( REMOVE_ITEM DEVHPPS ${CMAKE_CURRENT_SOURCE_DIR}/can/SocketCANController.hpp)
    LIST( REMOVE_ITEM DEVSRCS ${CMAKE_CURRENT_SOURCE_DIR}/can/RTCANController.cpp)
    LIST( REMOVE_ITEM DEVHPPS ${CMAKE_CURRENT_SOURCE_DIR}/can/RTCANController.hpp)

    # Build orocos-can lib. [universal]
    GLOBAL_ADD_COMPONENT( orocos-can ${DEVSRCS} INSTALL lib)
    GLOBAL_ADD_INCLUDE( ocl/can ${DEVHPPS} )

    # Build orocos-canpie lib. [HW specific]
    IF ( BUILD_CANPIE )
       GLOBAL_ADD_COMPONENT( orocos-canpie can/CANPieController.cpp drivers/canpie/canpie_lxrtlib.c INSTALL lib)
       GLOBAL_ADD_INCLUDE( ocl/can can/CANPieController.hpp )
    ENDIF ( BUILD_CANPIE )

    # Build orocos-pcan lib. [HW specific]
    IF ( BUILD_PCAN )
       GLOBAL_ADD_COMPONENT( orocos-pcan can/PCANController.cpp INSTALL lib)
       GLOBAL_ADD_INCLUDE( ocl/can can/PCANController.hpp )
       COMPONENT_ADD_DEPS( orocos-pcan orocos-can )
       COMPONENT_ADD_LIBS( orocos-pcan pcan )
       OROCOS_PKGCONFIG_LIBS( pcan )
    ENDIF ( BUILD_PCAN )

    # Build orocos-socket-can lib. [HW specific]
    IF ( BUILD_SOCKET_CAN )
       GLOBAL_ADD_COMPONENT( orocos-socket-can can/SocketCANController.cpp INSTALL lib)
       GLOBAL_ADD_INCLUDE( ocl/can can/SocketCANController.hpp )
       COMPONENT_ADD_DEPS( orocos-socket-can orocos-can )
    ENDIF ( BUILD_SOCKET_CAN )

    # Build orocos-socket-can lib. [HW specific]
    IF ( BUILD_RTSOCKET_CAN )
       GLOBAL_ADD_COMPONENT( orocos-rtsocket-can can/RTCANController.cpp INSTALL lib)
       GLOBAL_ADD_INCLUDE( ocl/can can/RTCANController.hpp )
       COMPONENT_ADD_DEPS( orocos-rtsocket-can orocos-can )
       COMPONENT_ADD_LIBS( orocos-rtsocket-can rtdm )
       OROCOS_PKGCONFIG_LIBS( orocos-rtsocket-can rtdm )
    ENDIF ( BUILD_RTSOCKET_CAN )

ENDIF ( BUILD_CAN )
