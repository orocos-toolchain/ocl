
#include <rtt/os/main.h>
#include <EthercatIO.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <reporting/FileReporting.hpp>
#include <rtt/PeriodicActivity.hpp>
#include <hardware/io/IOComponent.hpp>

using namespace RTT;
using namespace OCL;

int ORO_main(int argc, char** argv)
{
	EthercatIO io("EthercatIO");
	IOComponent iocomp(8, 8, "IOComponent");
	PeriodicActivity pa(ORO_SCHED_RT,OS::HighestPriority,0.01,io.engine());
	PeriodicActivity pa_io(ORO_SCHED_RT,OS::LowestPriority,0.01,iocomp.engine());
	TaskBrowser tb(&io);
	FileReporting fr("Reporting");

	io.addPeer(&fr);
	io.addPeer(&iocomp);

	iocomp.addAnalogInput("AIN1", io.getAnalogIn(), 0);
	iocomp.addAnalogInput("AIN2", io.getAnalogIn(), 1);

	iocomp.addAnalogOutput("AOUT1", io.getAnalogOut(), 0);
	iocomp.addAnalogOutput("AOUT2", io.getAnalogOut(), 1);

	iocomp.addDigitalOutput("DOUT1", io.getDigitalOut(), 0);
	iocomp.addDigitalOutput("DOUT2", io.getDigitalOut(), 1);

	iocomp.addDigitalInput("DIN1", io.getDigitalIn(), 0);
	iocomp.addDigitalInput("DIN2", io.getDigitalIn(), 1);

	tb.loop();
	return 0;
}
