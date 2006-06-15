//hardware interfaces

#include "Kuka160nAxesVelocityController.hpp"

//User interface
#include "../../taskbrowser/TaskBrowser.hpp"

//Reporting
#include "../../reporting/FileReporting.hpp"

//#include <corelib/Activities.hpp>
//#include <execution/GenericTaskContext.hpp>
//#include <corelib/Logger.hpp>
#include <os/main.h>

using namespace Orocos;
using namespace RTT;
using namespace std;

class EmergencyStop
{
public:
  EmergencyStop(GenericTaskContext *axes)
    : _axes(axes) {
    _stop = _axes->commands()->create("this", "stopAxis").arg(_axis).arg(_value);
    _lock = _axes->commands()->create("this", "lockAxis").arg(_axis).arg(_value);
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
  GenericTaskContext *_axes;
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

/// main() function

int ORO_main(int argc, char* argv[])
{

  if ( Logger::log().getLogLevel() < Logger::Info ) {
    Logger::log().setLogLevel( Logger::Info );
              Logger::log() << Logger::Info << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<Logger::endl;
  }
  
  Kuka160nAxesVelocityController my_robot("Kuka160");
  
  EmergencyStop _emergency(&my_robot);
  
  /// Creating Event Handlers
  Handle _emergencyHandle = my_robot.events()->setupConnection("driveOutOfRange").
    callback(&_emergency,&EmergencyStop::callback).handle();
  Handle _positionWarning = my_robot.events()->setupConnection("positionOutOfRange").
    callback(&PositionLimitCallBack).handle();

  /// Connecting Event Handlers
  _emergencyHandle.connect();
  _positionWarning.connect();
  
  ///Reporting
  FileReporting reporter("Reporting");
  reporter.addPeer(&my_robot);

  /// Link my_robot to Taskbrowser
  TaskBrowser browser(&my_robot );
  browser.setColorTheme( TaskBrowser::whitebg );
  browser.addPeer(&reporter);

  //Loading program in browser
  browser.loadProgram("cpf/program.ops");

  /// Creating Tasks
  NonPreemptibleActivity _kukaTask(0.1, my_robot.engine() );  
  PeriodicActivity _browserTask(2,1, browser.engine() );  
  PeriodicActivity reportingTask(10,1.0,reporter.engine());
  /// Start the console reader.
  browser.loop();
  
  _kukaTask.stop();

  return 0;
}
