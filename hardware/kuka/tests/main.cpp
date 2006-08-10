//hardware interfaces

#include "hardware/kuka/Kuka160nAxesVelocityController.hpp"
#include "hardware/kuka/Kuka361nAxesVelocityController.hpp"

//User interface
#include "taskbrowser/TaskBrowser.hpp"

//Reporting
#include "reporting/FileReporting.hpp"

//#include <rtt/Activities.hpp>
//#include <rtt/GenericTaskContext.hpp>
//#include <rtt/Logger.hpp>
#include <rtt/os/main.h>

using namespace Orocos;
using namespace RTT;
using namespace std;

class EmergencyStop
{
public:
  EmergencyStop(GenericTaskContext* axes)
    : _axes(axes),fired(6,false) {
      try{
          _stop = axes->commands()->getCommand<bool(int)>("stopAxis");
          _lock = axes->commands()->getCommand<bool(int)>("lockAxis");
      }catch(...){
          log(Error)<<"EmergencyStop: Failed to construct stop and lock Commands"<<endl;
          exit(-1);
      }
  };
    ~EmergencyStop(){};
    void callback(int axis, double value) {
      if(!fired[axis]){
          _stop(axis);
          _lock(axis);
          cout << "---------------------------------------------" << endl;
          cout << "--------- EMERGENCY STOP --------------------" << endl;
          cout << "---------------------------------------------" << endl;
          cout << "Axis "<< axis <<" drive value "<<value<< " reached limitDriveValue"<<endl;
          fired[axis] = true;
      }
  };
private:
  GenericTaskContext *_axes;
  Command<bool(int)> _stop;
  Command<bool(int)> _lock;
  int _axis;
  double _value;
    std::vector<bool> fired;
    
}; // class

void PositionLimitCallBack(int axis, double value)
{
  cout<< "-------------Warning----------------"<<endl;
  cout<< "Axis "<<axis<<" moving passed software position limit, current value: "<<value<<endl;
}

/// main() function

int ORO_main(int argc, char* argv[])
{
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
  
  if ( Logger::log().getLogLevel() < Logger::Info ) {
    Logger::log().setLogLevel( Logger::Info );
              Logger::log() << Logger::Info << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<Logger::endl;
  }
  
  
  EmergencyStop _emergency(my_robot);
  
  /// Creating Event Handlers
  Handle _emergencyHandle = my_robot->events()->setupConnection("driveOutOfRange").
    callback(&_emergency,&EmergencyStop::callback).handle();
  Handle _positionWarning = my_robot->events()->setupConnection("positionOutOfRange").
    callback(&PositionLimitCallBack).handle();

  /// Connecting Event Handlers
  _emergencyHandle.connect();
  _positionWarning.connect();
  
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
