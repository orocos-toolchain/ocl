# FIXME: Remove these absolute paths
CPPFLAGS+=-I/usr/realtime/include
CPPFLAGS+=-I/home/sspr/src/orocos-trunk/build/packages/install/include
CPPFLAGS+=-I/usr/src/linux

obj-m += cpsja.o

cpsja-objs := cplinux.o cpuser.o cpfifo.o cpmsg.o cpfilter.o cpcore_sja.o


VERBOSE=1

# type "make -C /usr/src/linux/ SUBDIRS=$PWD modules"