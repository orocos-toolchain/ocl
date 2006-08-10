//hardware interfaces

#include "hardware/kuka/Kuka160nAxesVelocityController.hpp"
#include "hardware/kuka/Kuka361nAxesVelocityController.hpp"

//User interface
#include "taskbrowser/TaskBrowser.hpp"

//Reporting
#include "reporting/FileReporting.hpp"

#include <rtt/os/main.h>

#include "EmergencyStop.hpp"

using namespace Orocos;
using namespace RTT;
using namespace std;

/// main() function

int ORO_main(int argc, char* argv[])
{
  if ( Logger::log().getLogLevel() < Logger::Debug ) {
    Logger::log().setLogLevel( Logger::Debug );
    log(Info) << argv[0] << " manually raises LogLevel to 'Debug'."
              << " See also file 'orocos.log'." << endlog();
  }

  GenericTaskContext* my_robot = NULL;
  if (argc > 1)
    {
      string s = argv[1];
      if(s == "Kuka361"){
	Logger::log()<<Logger::Warning<<"Choosing Kuka361"<<Logger::endl;
	my_robot = new Kuka361nAxesVelocityController("Kuka361");
      }
      else if(s == "Kuka160"){
	Logger::log()<<Logger::Warning<<"Choosing Kuka160"<<Logger::endl;
	my_robot = new Kuka160nAxesVelocityController("Kuka160");
      }
    }
  else{
    Logger::log()<<Logger::Warning<<"Using Default Kuka160"<<Logger::endl;
    my_robot = new Kuka160nAxesVelocityController("Kuka160");
  }
  
  EmergencyStop _emergency(my_robot);
  
  /// Creating Event Handlers
  _emergency.addEvent(my_robot,"driveOutOfRange");
  _emergency.addEvent(my_robot,"positionOutOfRange");
  
  /// Link my_robot to Taskbrowser
  TaskBrowser browser(my_robot );
  browser.setColorTheme( TaskBrowser::whitebg );

  //Loading program in browser
  my_robot->loadProgram("cpf/program.ops");

  /// Creating Tasks
  NonPreemptibleActivity _kukaTask(0.1, my_robot->engine() );  
  
  /// Start the console reader.
  _kukaTask.start();

  browser.loop();
  
  return 0;
}
