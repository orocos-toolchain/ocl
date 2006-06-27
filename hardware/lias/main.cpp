//hardware interfaces
#include <corelib/ZeroTimeThread.hpp>
#include "LiASnAxesVelocityController.hpp"

//User interface
#include "../../taskbrowser/TaskBrowser.hpp"

//Reporting
#include "../../reporting/FileReporting.hpp"

#include <corelib/Activities.hpp>
#include <execution/GenericTaskContext.hpp>
#include <corelib/Logger.hpp>
#include <os/main.h>

#include <execution/TemplateFactories.hpp>
#include <string>

using namespace Orocos;
using namespace RTT;
using namespace std;


class Supervisor : public GenericTaskContext
{
public:
   Supervisor():
	GenericTaskContext("supervisor") ,
    driveValue(6)	
	{
  		TemplateMethodFactory<Supervisor>* cmeth = newMethodFactory( this );
  		cmeth->add( "message",  method( &Supervisor::message,  "give a message to the user","msg","msg to display" ) );
  		this->methods()->registerObject("this", cmeth);
		char buf[80];
		for (int i=0;i<6;++i) {
			sprintf(buf,"driveValue%d",i);
			driveValue[i] = new WriteDataPort<double>(buf);
			ports()->addPort(driveValue[i]);
		}
	}

   virtual void message(const std::string& msg) {
         Logger::log() << Logger::Info << msg <<Logger::endl;
	}

   virtual ~Supervisor() {}
   virtual bool startup() {
	   return true;
   }
                   
   virtual void update() {
   }
 
   virtual void shutdown() {
   }

	std::vector<ORO_Execution::WriteDataPort<double>*> driveValue;	
};


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
  ZeroTimeThread::Instance()->stop();
  ZeroTimeThread::Instance()->setPeriod(0.002);
  ZeroTimeThread::Instance()->start();


  if ( Logger::log().getLogLevel() < Logger::Info ) {
    Logger::log().setLogLevel( Logger::Info );
              Logger::log() << Logger::Info << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<Logger::endl;
  }
  
  GenericTaskContext* my_robot = new LiASnAxesVelocityController("lias");
  EmergencyStop _emergency(my_robot);
  Supervisor supervisor;


  /// Creating Event Handlers
  Handle _emergencyHandle = my_robot->events()->setupConnection("driveOutOfRange").
    callback(&_emergency,&EmergencyStop::callback).handle();
  Handle _positionWarning = my_robot->events()->setupConnection("positionOutOfRange").
    callback(&PositionLimitCallBack).handle();

  /// Connecting Event Handlers
  _emergencyHandle.connect();
  _positionWarning.connect();
  
  /// Link my_robot to Taskbrowser
  TaskBrowser browser( my_robot );
  browser.setColorTheme( TaskBrowser::whitebg );
  
  ///Reporting
  FileReporting reporter("Reporting");

  // Connecting to peers
  reporter.connectPeers(my_robot);
  supervisor.connectPeers(my_robot);
  
  /// Creating Task
  NonPreemptibleActivity _robotTask(0.002, my_robot->engine() ); 
  NonPreemptibleActivity _supervisorTask(0.002, supervisor.engine() ); 
  PeriodicActivity       reportingTask(10,0.01,reporter.engine());

  // Load some default programs :
  my_robot->loadProgram("cpf/program.ops"); 
  /// Start the console reader.
  browser.loop();
  Logger::log()<< Logger::Info << "Browser ended " << Logger::endl;
  
  _robotTask.stop();
  Logger::log()<< Logger::Info << "Task stopped" << Logger::endl;
  delete my_robot;
  Logger::log()<< Logger::Info << "robot deleted" << Logger::endl;
  return 0;
}
