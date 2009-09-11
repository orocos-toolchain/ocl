#include <rtt/RTT.hpp>
#include <rtt/Activities.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/os/main.h>

#include <taskbrowser/TaskBrowser.hpp>
#include <io/IOComponent.hpp>

#include <comedi/dev/ComediDevice.hpp>
#include <comedi/dev/ComediSubDeviceAOut.hpp>
#include <comedi/dev/ComediSubDeviceDIn.hpp>
#include <comedi/dev/ComediSubDeviceDOut.hpp>

using namespace Orocos;

int ORO_main(int arc, char* argv[])
{
  //Creating IOComponent and its activity
  IOComponent io("IO");
  PeriodicActivity io_act(ORO_SCHED_RT,RTT::OS::HighestPriority, 0.001);
  io_act.run(io.engine());

  //Creating ComediDevices:
  //ComediDevice* AOut = new ComediDevice(0);
  ComediDevice* Encoder = new ComediDevice(2);

  //ComediSubDeviceAOut SubAOut(AOut,"AnalogOut",1);
  ComediSubDeviceDOut SubDOut(Encoder,"DigitalOut",1);
  //ComediSubDeviceDOut SubDOut(DInOut,"DigitalOut",1);

  //Adding Hardware interfaces to the IOComponent
  io.addDigitalOutput("D0","DigitalOut",0);
  io.addDigitalOutput("D1","DigitalOut",1);
  io.addDigitalOutput("D2","DigitalOut",2);
  io.addDigitalOutput("D3","DigitalOut",3);
  io.addDigitalOutput("D4","DigitalOut",4);
  io.addDigitalOutput("D5","DigitalOut",5);
  io.addDigitalOutput("D6","DigitalOut",6);
  io.addDigitalOutput("D7","DigitalOut",7);
  io.addDigitalOutput("D8","DigitalOut",8);
  io.addDigitalOutput("D9","DigitalOut",9);

  //Creating the TaskBrowser:
  TaskBrowser tb(&io);

  tb.loop();

  //delete AOut;
  //delete DInOut;
  delete Encoder;

  return 0;
}




