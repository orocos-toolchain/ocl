KERNEL_DIR=/lib/modules/2.6.15.7/build/
RTAI_INSTALL_DIR=/usr/realtime/rtai-3.3/
RTNET_INSTALL_DIR=/usr/realtime/rtnet-0.9.4-rtai-3.3/

CPPFLAGS+=-I$(RTAI_INSTALL_DIR)/include -I$(RTNET_INSTALL_DIR)/include
CFLAGS+=-march=pentium4 -mcpu=pentium4

obj-m := krypton_handler.o


default:
	@cat README

driver:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

