DEPENDENT_OPTION( BUILD_CAN "Build Can Component" OFF "BUILD_HARDWARE" OFF)
DEPENDENT_OPTION( BUILD_CANPIE "Build Canpie Controller (LXRT only)" OFF "BUILD_CAN;OS_LXRT" OFF)


IF ( BUILD_CAN )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB DEVSRCS can/[^.]*.cpp )
    FILE( GLOB DEVHPPS can/[^.]*.hpp )

    LIST( REMOVE_ITEM DEVSRCS ${CMAKE_CURRENT_SOURCE_DIR}/can/CANPieController.cpp)
    LIST( REMOVE_ITEM DEVHPPS ${CMAKE_CURRENT_SOURCE_DIR}/can/CANPieController.hpp)

    # Build orocos-canpie lib. [HW specific]
    IF ( BUILD_CANPIE )
       GLOBAL_ADD_COMPONENT( orocos-canpie can/CANPieController.cpp drivers/canpie/canpie_lxrtlib.c)
       GLOBAL_ADD_INCLUDE( ocl/can can/CANPieController.hpp )
    ENDIF ( BUILD_CANPIE )

    # Build orocos-can lib. [universal]
    GLOBAL_ADD_COMPONENT( orocos-can ${DEVSRCS} )
    GLOBAL_ADD_INCLUDE( ocl/can ${DEVHPPS} )

    #ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_CAN )
