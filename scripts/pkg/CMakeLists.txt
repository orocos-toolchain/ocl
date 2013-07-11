if (NOT IS_ROS_PACKAGE)
  INSTALL( PROGRAMS orocreate-pkg DESTINATION bin )
  INSTALL( DIRECTORY templates DESTINATION share/ocl )
endif()