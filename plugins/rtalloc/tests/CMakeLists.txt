IF (BUILD_TESTING)

  ADD_EXECUTABLE(rtalloc-combined combined.cpp send.cpp recv.cpp)
  TARGET_LINK_LIBRARIES(rtalloc-combined
	orocos-taskbrowser-${OROCOS_TARGET}
	orocos-rtalloctk-${OROCOS_TARGET}
	orocos-rtt-${OROCOS_TARGET}
	)

ENDIF (BUILD_TESTING)
