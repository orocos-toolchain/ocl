IF (BUILD_TESTING AND BUILD_RTALLOC_TEST)

  # Workaround for RTT pkgconfig file issue
  find_package(Boost 1.32 COMPONENTS thread REQUIRED)

  ADD_EXECUTABLE(rtalloc-corba-combined 
	corba-combined.cpp 
	../../tests/send.cpp 
	../../tests/recv.cpp)
  TARGET_LINK_LIBRARIES(rtalloc-corba-combined
	orocos-rtalloctk-corba-${OROCOS_TARGET}
	orocos-rtt-${OROCOS_TARGET}
	# for some reason, this rtalloc library must be explicitly listed. SNR
	${Boost_THREAD_LIBRARY}
	)

  ADD_EXECUTABLE(rtalloc-corba-recv 
	corba-recv.cpp 
	../../tests/recv.cpp)
  TARGET_LINK_LIBRARIES(rtalloc-corba-recv
	orocos-rtalloctk-corba-${OROCOS_TARGET}
	orocos-taskbrowser-${OROCOS_TARGET}
	orocos-rtt-${OROCOS_TARGET}
	)

  ADD_EXECUTABLE(rtalloc-corba-send 
	corba-send.cpp 
	../../tests/send.cpp)
  TARGET_LINK_LIBRARIES(rtalloc-corba-send
	orocos-rtalloctk-corba-${OROCOS_TARGET}
	orocos-taskbrowser-${OROCOS_TARGET}
	orocos-rtt-${OROCOS_TARGET}
	)

ENDIF (BUILD_TESTING AND BUILD_RTALLOC_TEST)
