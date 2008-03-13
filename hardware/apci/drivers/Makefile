KERNEL_DIR=/usr/src/linux-headers-2.6.19.7-rtai-ipipe-1.7-01
RTAI_INSTALL_DIR=/usr/realtime/

CPPFLAGS+=-I$(RTAI_INSTALL_DIR)/include
#CFLAGS+=-march=pentium4 -mcpu=pentium4
#CC=gcc-4.0

obj-m := 

all:
	@cat README

driver:
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) CC=$(CC) modules
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci1710 CC=$(CC) modules
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci1032 CC=$(CC) modules
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci2200 CC=$(CC) modules
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/lxrt CC=$(CC) modules LXRT_FLAGS="-I$(RTAI_INSTALL_DIR)/include"

clean:
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci1710 clean
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci1032 clean
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci2200 clean
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/lxrt clean


# type "make -C /usr/src/linux-2.6.9_adeos-r9c3/ SUBDIRS=$PWD modules"
