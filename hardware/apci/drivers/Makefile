KERNEL_DIR=/usr/src/linux-2.6.15.7-rtai-3.3
RTAI_INSTALL_DIR=/usr/realtime/rtai-3.3

#CPPFLAGS+=-I$(RTAI_INSTALL_DIR)/include
#CFLAGS+=-march=pentium4 -mcpu=pentium4

obj-m := 

all:
	@cat README

driver:
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci1710 modules
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci1032 modules
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci2200 modules
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/lxrt modules LXRT_FLAGS="-I$(RTAI_INSTALL_DIR)/include"

clean:
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci1710 clean
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci1032 clean
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/apci2200 clean
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD)/lxrt clean


# type "make -C /usr/src/linux-2.6.9_adeos-r9c3/ SUBDIRS=$PWD modules"
