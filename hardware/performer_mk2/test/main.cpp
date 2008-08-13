//hardware interfaces

#include <hardware/performer_mk2/PerformernAxesVelocityController.hpp>
#include <hardware/kuka/EmergencyStop.hpp>


//User interface
#include <taskbrowser/TaskBrowser.hpp>

//Reporting
#include <reporting/FileReporting.hpp>

#include <rtt/os/main.h>
#include <rtt/PeriodicActivity.hpp>

using namespace Orocos;
using namespace RTT;
using namespace std;

/// main() function

int ORO_main(int argc, char* argv[])
{

  PerformerMK2nAxesVelocityController my_robot("Performer");
  log(Info)<<"Robot Created"<<endlog();

  EmergencyStop _emergency(&my_robot);

  /// Creating Event Handlers
  _emergency.addEvent(&my_robot,"driveOutOfRange");

  /// Link my_robot to Taskbrowser
  TaskBrowser browser(&my_robot );

  /// Creating Tasks
  PeriodicActivity robotTask(0,0.002, my_robot.engine() );

  /// Start the console reader.
  robotTask.start();

  browser.loop();

  return 0;
}
