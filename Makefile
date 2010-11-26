ifdef ROS_ROOT
include $(shell rospack find mk)/cmake.mk
else
$(warning This Makefile only works with ROS rosmake, without rosmake do the normal cmake stuff)
endif
