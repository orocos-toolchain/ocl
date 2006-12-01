DEPENDENT_OPTION( BUILD_KUKA "Build Kuka Components" OFF  "BUILD_HARDWARE" OFF)
#DEPENDENT_OPTION( BUILD_KUKA_HW "Build Kuka HW Components" ON  "BUILD_KUKA;BUILD_LXRT;BUILD_APCI;BUILD_IO;BUILD_COMEDI" OFF)

#Always process subdirs
ADD_SUBDIRECTORY( tests )

IF ( BUILD_KUKA )
    INCLUDE_DIRECTORIES( ${PROJ_SOURCE_DIR}/hardware/axes ${PROJ_SOURCE_DIR}/hardware/comedi ${PROJ_SOURCE_DIR}/hardware/apci )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-kuka ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

ENDIF ( BUILD_KUKA )
