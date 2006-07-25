
#OROPATH =/src/orocos-trunk/build-gnu/packages/install
OROPATH =/src/orocos-trunk/build-rtai/packages/install
LXRTPATH=/usr/realtime/rtai-magma_linux-2.6.9_hal-r10

CC = g++
LD = ld

OROFLAGS= -Wall -g -I${OROPATH}/include
OROLIBS = -lm -lxerces-c -lpthread -lreadline -lncurses -lACE

all: wrenchtask-lxrt

WrenchSensor.o: WrenchSensor.cpp ${OROPATH}/include/os/gnulinux.h
	${CC} -c ${OROFLAGS} $< -o $@

main.o: main.cpp ${OROPATH}/include/os/gnulinux.h
	${CC} -c ${OROFLAGS} $< -o $@

wrenchtask: WrenchSensor.o main.o  ${OROPATH}/lib/liborocos-gnulinux.a
	${CC} -o $@ main.o WrenchSensor.o -L${OROPATH}/lib -lorocos-gnulinux ${OROLIBS}

WrenchSensor-lxrt.o: WrenchSensor.cpp ${OROPATH}/include/os/lxrt.h
	${CC} -c ${OROFLAGS} $< -o $@

main-lxrt.o: main.cpp ${OROPATH}/include/os/lxrt.h
	${CC} -c ${OROFLAGS} $< -o $@

wrenchtask-lxrt: WrenchSensor.o main.o ${OROPATH}/lib/liborocos-lxrt.a
	${CC} -o $@ main.o WrenchSensor.o -L${OROPATH}/lib -L$(LXRTPATH)/lib -L/usr/src/rtai/lxrt/lib -lorocos-lxrt -llxrt ${OROLIBS}


clean:
	rm -f *.o
	rm -f simpletask simpletask-lxrt

FILES= README SimpleTask.cxx cpf.dtd Makefile
dist:
	cd .. ;\
	tar -cvzf simple-task.tar.gz $(addprefix task-intro/,$(FILES))
