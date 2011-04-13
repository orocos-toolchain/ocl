if (NOT ROS_ROOT)
  INSTALL( PROGRAMS orocreate-pkg DESTINATION bin )
  INSTALL( DIRECTORY templates DESTINATION share/orocos-ocl )
endif()