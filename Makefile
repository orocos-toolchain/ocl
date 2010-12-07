ifdef ROS_ROOT
include $(shell rospack find mk)/cmake.mk
else
$(warning This Makefile only works with ROS rosmake. Without rosmake, create a build directory and run cmake ..)
endif
