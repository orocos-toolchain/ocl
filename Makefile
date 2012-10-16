ifdef ROS_ROOT
default: install
include $(shell rospack find mk)/cmake.mk
include $(shell rospack find rtt)/../env.mk
EXTRA_CMAKE_FLAGS=-DCMAKE_INSTALL_PREFIX=`rospack find rtt`/../install -DBUILD_TESTING=OFF
install: all
	cd build; ${MAKE} install
else
$(warning This Makefile only works with ROS rosmake. Without rosmake, create a build directory and run cmake ..)
endif
