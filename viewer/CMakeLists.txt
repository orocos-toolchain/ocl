DEPENDENT_OPTION( BUILD_VIEWER "Build Viewer Component" OFF "" OFF)

ADD_SUBDIRECTORY( tests )
ADD_SUBDIRECTORY( cpf )

IF ( BUILD_VIEWER )

    SET(SRCS naxespositionviewer.cpp naxespositionviewer.hpp)
    
    FILE( GLOB HPPS [^.]*.hpp )
    GLOBAL_ADD_COMPONENT( orocos-viewer ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

ENDIF ( BUILD_VIEWER )
