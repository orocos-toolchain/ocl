#include <rtt/RTT.hpp>
#include <rtt/Activities.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/os/main.h>

#include <ocl/TaskBrowser.hpp>
#include <ocl/IOComponent.hpp>

#include <ocl/dev/ComediDevice.hpp>
#include <ocl/dev/ComediSubDeviceAOut.hpp>
#include <ocl/dev/ComediSubDeviceDIn.hpp>
#include <ocl/dev/ComediSubDeviceDOut.hpp>

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
  io.addDigitalOutput("D0",&SubDOut,0);
  io.addDigitalOutput("D1",&SubDOut,1);
  io.addDigitalOutput("D2",&SubDOut,2);
  io.addDigitalOutput("D3",&SubDOut,3);
  io.addDigitalOutput("D4",&SubDOut,4);
  io.addDigitalOutput("D5",&SubDOut,5);
  io.addDigitalOutput("D6",&SubDOut,6);
  io.addDigitalOutput("D7",&SubDOut,7);
  io.addDigitalOutput("D8",&SubDOut,8);
  io.addDigitalOutput("D9",&SubDOut,9);

  //Creating the TaskBrowser:
  TaskBrowser tb(&io);

  tb.loop();

  //delete AOut;
  //delete DInOut;
  delete Encoder;

  return 0;
}




