KERNEL_DIR=/lib/modules/2.6.15.7/build/
RTAI_INSTALL_DIR=/usr/realtime/rtai-3.3/

CPPFLAGS+=-I$(RTAI_INSTALL_DIR)/include
CFLAGS+=-march=pentium4 -mcpu=pentium4

obj-m := jr3dsp.o jr3_lxrt_kernel.o


all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
