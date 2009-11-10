OPTION( BUILD_RTALLOC "Build real-time memory allocation" OFF )

GLOBAL_ADD_INCLUDE( ocl String.hpp )

IF ( BUILD_RTALLOC )

  ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_RTALLOC )

