OPTION( BUILD_RTALLOC "Build real-time memory allocation" OFF )

IF ( BUILD_RTALLOC )

  ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_RTALLOC )

