//hardware interfaces
#include "../../hardware/kuka/Kuka160nAxesVelocityController.hpp"

//User interface
#include "../../taskbrowser/TaskBrowser.hpp"

//Reporting
#include "../../reporting/FileReporting.hpp"

//nAxes components
#include "nAxesComponents.hpp"

//#include <corelib/Activities.hpp>
#include <execution/GenericTaskContext.hpp>
//#include <corelib/Logger.hpp>
#include <os/main.h>

using namespace Orocos;
using namespace RTT;
using namespace std;

class EmergencyStop
{
public:
  EmergencyStop(GenericTaskContext *robot)
    : _robot(robot) {
    _stop = _robot->commands()->create("this", "stopAxis").arg(_axis).arg(_value);
    _lock = _robot->commands()->create("this", "lockAxis").arg(_axis).arg(_value);
  };
  ~EmergencyStop(){};
  void callback(int axis, double value) {
    _axis = axis;
    _value = value;
    _stop.execute();
    _lock.execute();
    cout << "---------------------------------------------" << endl;
    cout << "--------- EMERGENCY STOP --------------------" << endl;
    cout << "---------------------------------------------" << endl;
    cout << "Axis "<< _axis <<" drive value "<<_value<< " reached limitDriveValue"<<endl;
  };
private:
  GenericTaskContext *_robot;
  CommandC _stop;
  CommandC _lock;
  int _axis;
  double _value;
}; // class

void PositionLimitCallBack(int axis, double value)
{
  cout<< "-------------Warning----------------"<<endl;
  cout<< "Axis "<<axis<<" moving passed software position limit, current value: "<<value<<endl;
}

// main() function

int ORO_main(int argc, char* argv[])
{

  if ( Logger::log().getLogLevel() < Logger::Info ) {
    Logger::log().setLogLevel( Logger::Info );
              Logger::log() << Logger::Info << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<Logger::endl;
  }
  
  Kuka160nAxesVelocityController my_robot("Kuka160");

  EmergencyStop _emergency(&my_robot);
  
  // Creating Event Handlers
  Handle _emergencyHandle = my_robot.events()->setupConnection("driveOutOfRange").
    callback(&_emergency,&EmergencyStop::callback).handle();
  Handle _positionWarning = my_robot.events()->setupConnection("positionOutOfRange").
    callback(&PositionLimitCallBack).handle();

  // Connecting Event Handlers
  _emergencyHandle.connect();
  _positionWarning.connect();
  
  //nAxesComponents
  nAxesSensorPos _sensor("nAxesSensor",6);
  nAxesGeneratorPos _generator("nAxesGenerator",6);
  nAxesControllerPosVel _controller("nAxesController",6);
  nAxesEffectorVel _effector("nAxesEffector",6);

  //connecting sensor and effector to hardware
  _sensor.addPeer(&my_robot);
  _effector.addPeer(&my_robot);
  
  //connection naxes components to each other
  _generator.connectPeers(&_sensor);
  _controller.connectPeers(&_sensor);
  _controller.connectPeers(&_generator);
  _effector.connectPeers(&_controller);
    
  //Reporting
  FileReporting reporter("Reporting");
  reporter.connectPeers(&_sensor);
  reporter.connectPeers(&_generator);
  reporter.connectPeers(&_controller);
  reporter.connectPeers(&_effector);
    
  
  // Creating Tasks
  NonPreemptibleActivity _kukaTask(0.01, my_robot.engine() ); 
  NonPreemptibleActivity _sensorTask(0.1, _sensor.engine() ); 
  NonPreemptibleActivity _generatorTask(0.1, _generator.engine() ); 
  NonPreemptibleActivity _controllerTask(0.1, _controller.engine() ); 
  NonPreemptibleActivity _effectorTask(0.1, _effector.engine() ); 
  PeriodicActivity reportingTask(10,1.0,reporter.engine());

  // Link my_robot to Taskbrowser
  TaskBrowser browser(&my_robot);
  browser.setColorTheme( TaskBrowser::whitebg );
  
  // Start the console reader.
  browser.loop();
  
  return 0;
}
