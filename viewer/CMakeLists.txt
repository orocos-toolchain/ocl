OPTION( BUILD_VIEWER "Build Viewer Component" OFF )

IF ( BUILD_VIEWER )

    SET(SRCS naxespositionviewer.cpp naxespositionviewer.hpp)

    GLOBAL_ADD_COMPONENT( orocos-viewer ${SRCS} )

    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_VIEWER )
