#include <rtt/ZeroTimeThread.hpp>
#include <rtt/Activities.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/os/main.h>

#include "viewer/naxespositionviewer.hpp"
//#include "hardware/lias/LiASnAxesVelocityController.hpp"
#include "hardware/kuka/Kuka160nAxesVelocityController.hpp"
#include "taskbrowser/TaskBrowser.hpp"
#include "reporting/FileReporting.hpp"

#include <string>

using namespace RTT;
using namespace std;
using namespace Orocos;


class Supervisor : public TaskContext
{
	int nrofaxes;
public:
   Supervisor(int _nrofaxes=6):
	TaskContext("supervisor") ,
	nrofaxes(_nrofaxes),
    driveValue(_nrofaxes),
	reference(_nrofaxes)
	{
  		this->methods()->addMethod( method("message",  &Supervisor::message,this),  "give a message to the user","msg","msg to display" );
  		this->methods()->addMethod( method( "setDriveValue",&Supervisor::setDriveValue, this), 
					"sets the value of a driveValue port",
					"axis","the driveValue port for this axis",
					"value","the drive value in rad/s" );
		this->methods()->addMethod( method("getReference",  &Supervisor::getReference,this),
					"gets the reference switch value from a reference port",
					"axis","the reference port corresponding to this axis");
		char buf[80];
		for (int i=0;i<_nrofaxes;++i) {
			sprintf(buf,"driveValue%d",i);
			driveValue[i] = new WriteDataPort<double>(buf);
			ports()->addPort(driveValue[i]);
			sprintf(buf,"reference%d",i);
			reference[i] = new ReadDataPort<bool>(buf);
			ports()->addPort(reference[i]);
		}
	}

   virtual void message(const std::string& msg) {
         Logger::log() << Logger::Info << msg <<Logger::endl;
	}
   virtual void setDriveValue(int axis,double value) {
		if ((0<=axis)&&(axis<nrofaxes)) {
			driveValue[axis]->Set( value);
		} else {
  			Logger::log()<< Logger::Error << "parameter axis out of range" << Logger::endl;
		}
   }
   virtual bool getReference(int axis) {
		if ((0<=axis)&&(axis<nrofaxes)) {
			return reference[axis]->Get();
		} else {
  			Logger::log()<< Logger::Error << "parameter axis out of range" << Logger::endl;
            return false;
		}
   }

   virtual ~Supervisor() {}
   virtual bool startup() {
	   return true;
   }
                   
   virtual void update() {
   }
 
   virtual void shutdown() {
		for (int i=0;i<nrofaxes;++i) {
			driveValue[i]->Set( 0.0 );
		}
   }

	std::vector<RTT::WriteDataPort<double>*> driveValue;	
	std::vector<RTT::ReadDataPort<bool>*>  reference;	
};


class EmergencyStop
{
public:
  EmergencyStop(GenericTaskContext *axes)
    : _axes(axes) {
    _stop = _axes->commands()->getCommand<bool(int,double)>("stopAxis");
    _lock = _axes->commands()->getCommand<bool(int,double)>("lockAxis");
  };
  ~EmergencyStop(){};
  void callback(int axis, double value) {
    _axis = axis;
    _value = value;
    _stop.execute();
    _lock.execute();
    log(Error) << "---------------------------------------------" << endlog();
    log(Error) << "--------- EMERGENCY STOP --------------------" << endlog();
    log(Error) << "---------------------------------------------" << endlog();
    log(Error) << "Axis "<< _axis <<" drive value "<<_value<< " reached limitDriveValue"<<endlog();
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
  log(Warning)<< "-------------Warning----------------"<<endlog();
  log(Warning)<< "Axis "<<axis<<" moving passed software position limit, current value: "<<value<<endlog();
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
  GenericTaskContext* my_robot = new Kuka160nAxesVelocityController("lias"); 
  //GenericTaskContext* my_robot = new LiASnAxesVelocityController("lias");
  NAxesPositionViewer viewer("viewer","cpf/viewer.cpf");
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
   ///Reporting
  FileReporting reporter("Reporting");

  // Connecting to peers
  my_robot->connectPeers(&reporter);
  my_robot->connectPeers(&supervisor);
  my_robot->connectPeers(&viewer);
   
  /// Link my_robot to Taskbrowser
  TaskBrowser browser( my_robot );
  browser.setColorTheme( TaskBrowser::whitebg );
  

  /// Creating Task
  NonPreemptibleActivity _robotTask(0.002, my_robot->engine() ); 
  NonPreemptibleActivity _supervisorTask(0.002, supervisor.engine() ); 
  PeriodicActivity       _reportingTask(10,0.01,reporter.engine());
  NonRealTimeActivity    _viewerTask(0.1,viewer.engine());

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
