//hardware interfaces

#include <hardware/kuka/Kuka361nAxesTorqueController.hpp>
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
//  if ( Logger::log().getLogLevel() < Logger::Debug ) {
//    Logger::log().setLogLevel( Logger::Debug );
//    log(Info) << argv[0] << " manually raises LogLevel to 'Debug'."
//              << " See also file 'orocos.log'." << endlog();
//  }

  TaskContext* my_robot = new Kuka361nAxesTorqueController("Kuka361");

  EmergencyStop _emergency(my_robot);

  /// Creating Event Handlers
  _emergency.addEvent(my_robot,"positionOutOfRange");
  _emergency.addEvent(my_robot,"velocityOutOfRange");
  _emergency.addEvent(my_robot,"currentOutOfRange");

  /// Link my_robot to Taskbrowser
  TaskBrowser browser(my_robot );
  browser.setColorTheme( TaskBrowser::whitebg );

  //Loading program in browser
  my_robot->scripting()->loadPrograms("cpf/program.ops");

  /// Creating Tasks
  PeriodicActivity _kukaTask(OS::HighestPriority,0.01, my_robot->engine() );

   //Reporting
   FileReporting reporter("Reporting");
   reporter.marshalling()->updateProperties("cpf/reporter.cpf");
   reporter.connectPeers(my_robot);
   PeriodicActivity _reportingTask(3,0.01,reporter.engine());

  /// Start the console reader.
  _kukaTask.start();
  reporter.configure();
  _reportingTask.start();

  browser.loop();

  return 0;
}
