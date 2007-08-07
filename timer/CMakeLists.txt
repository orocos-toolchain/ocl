OPTION( BUILD_TIMER "Build Timer Component" ON )

IF ( BUILD_TIMER )

  # Better inform user instead of hiding the option.
  IF(NOT OROCOS_RTT_1.2 )
    MESSAGE(FATAL_ERROR "Timer Component requires Orocos RTT 1.4.0 or newer." )
  ENDIF(NOT OROCOS_RTT_1.2 )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB HPPS [^.]*.hpp )
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-timer ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})
     
    ADD_SUBDIRECTORY( tests )

ENDIF ( BUILD_TIMER )
