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
    Logger::log()<<Logger::Error << "---------------------------------------------" << Logger::endl;
    Logger::log()<<Logger::Error << "--------- EMERGENCY STOP --------------------" << Logger::endl;
    Logger::log()<<Logger::Error << "---------------------------------------------" << Logger::endl;
    Logger::log()<<Logger::Error << "Axis "<< _axis <<" drive value "<<_value<< " reached limitDriveValue"<<Logger::endl;
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
  Logger::log()<<Logger::Warning<< "-------------Warning----------------"<<Logger::endl;
  Logger::log()<<Logger::Warning<< "Axis "<<axis<<" moving passed software position limit, current value: "<<value<<Logger::endl;
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
  nAxesSensorPos sensor("nAxesSensor",6);
  nAxesGeneratorPos generatorPos("nAxesGeneratorPos",6);
  nAxesGeneratorVel generatorVel("nAxesGeneratorVel",6);
  nAxesControllerPos controllerPos("nAxesControllerPos",6);
  nAxesControllerPosVel controllerPosVel("nAxesControllerPosVel",6);
  nAxesControllerVel controllerVel("nAxesControllerPosVel",6);
  nAxesEffectorVel effector("nAxesEffector",6);

  //connecting sensor and effector to hardware
  sensor.connectPeers(&my_robot);
  effector.connectPeers(&my_robot);
  
  //connection naxes components to each other
  generatorPos.connectPeers(&sensor);
  generatorVel.connectPeers(&sensor);
  controllerPos.connectPeers(&sensor);
  controllerPosVel.connectPeers(&sensor);
  controllerVel.connectPeers(&sensor);
  controllerPos.connectPeers(&generatorPos);
  controllerPosVel.connectPeers(&generatorPos);
  controllerVel.connectPeers(&generatorVel);
  effector.connectPeers(&controllerPos);
  effector.connectPeers(&controllerPosVel);
  effector.connectPeers(&controllerVel);
    
  //Reporting
  FileReporting reporter("Reporting");
  reporter.connectPeers(&sensor);
  reporter.connectPeers(&generatorPos);
  reporter.connectPeers(&generatorVel);
  reporter.connectPeers(&controllerPos);
  reporter.connectPeers(&controllerPosVel);
  reporter.connectPeers(&controllerVel);
  reporter.connectPeers(&effector);  
    
  // Link my_robot to Taskbrowser
  TaskBrowser browser(&my_robot);
  browser.connectPeers(&reporter);
  browser.connectPeers(&sensor);    
  browser.connectPeers(&generatorPos); 
  browser.connectPeers(&generatorVel); 
  browser.connectPeers(&controllerPos);
  browser.connectPeers(&controllerPosVel);
  browser.connectPeers(&controllerVel);
  browser.connectPeers(&effector);
  
  browser.setColorTheme( TaskBrowser::whitebg );
  
  // Creating Tasks
  NonPreemptibleActivity _kukaTask(0.01, my_robot.engine() ); 
  NonPreemptibleActivity _sensorTask(0.1, sensor.engine() ); 
  NonPreemptibleActivity _generatorPosTask(0.1, generatorPos.engine() ); 
  NonPreemptibleActivity _generatorVelTask(0.1, generatorVel.engine() ); 
  NonPreemptibleActivity _controllerPosTask(0.1, controllerPos.engine() ); 
  NonPreemptibleActivity _controllerPosVelTask(0.1, controllerPosVel.engine() ); 
  NonPreemptibleActivity _controllerVelTask(0.1, controllerVel.engine() ); 
  NonPreemptibleActivity _effectorTask(0.1, effector.engine() ); 
  PeriodicActivity reportingTask(10,1.0,reporter.engine());
  PeriodicActivity browserTask(2,0.1,reporter.engine());

  browserTask.start();
  
  // Start the console reader.
  browser.loop();
  
  browserTask.stop();
  
  return 0;
}
