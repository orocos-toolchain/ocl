//hardware interfaces
#include <hardware/kuka/Kuka160nAxesVelocityController.hpp>

//User interface
#include <taskbrowser/TaskBrowser.hpp>

//Reporting
#include <reporting/FileReporting.hpp>

//Viewer
#include <viewer/naxespositionviewer.hpp>

//Cartesian components
#include <motion_control/cartesian/CartesianComponents.hpp>

//Kinematics component
#include <kdl/kinfam/kuka160.hpp>
#include <kdl/toolkit.hpp>
#include <kdl/kinfam/kinematicfamily_io.hpp>

#include <rtt/GenericTaskContext.hpp>
#include <rtt/os/main.h>

using namespace Orocos;
using namespace RTT;
using namespace KDL;
using namespace std;

class EmergencyStop
{
public:
    EmergencyStop(GenericTaskContext *robot)
        : _robot(robot),fired(6,false) {
        _stop = _robot->commands()->getCommand<bool(int,double)>("stopAxis");
        _lock = _robot->commands()->getCommand<bool(int,double)>("lockAxis");
    };
    ~EmergencyStop(){};
    void callback(int axis, double value) {
        if(!fired[axis]){
            _stop(axis,value);
            _lock(axis,value);
            Logger::log()<<Logger::Error << "---------------------------------------------" << Logger::endl;
            Logger::log()<<Logger::Error << "--------- EMERGENCY STOP --------------------" << Logger::endl;
            Logger::log()<<Logger::Error << "---------------------------------------------" << Logger::endl;
            Logger::log()<<Logger::Error << "Axis "<< axis <<" drive value "<<value<< " reached limitDriveValue"<<Logger::endl;
            fired[axis] = true;
        }
    };
private:
    GenericTaskContext *_robot;
    Command<bool(int,double)> _stop;
    Command<bool(int,double)> _lock;
    vector<bool> fired;
}; // class

void PositionLimitCallBack(int axis, double value)
{
    Logger::log()<<Logger::Warning<< "-------------Warning----------------"<<Logger::endl;
    Logger::log()<<Logger::Warning<< "Axis "<<axis<<" moving passed software position limit, current value: "<<value<<Logger::endl;
}

// main() function

int ORO_main(int argc, char* argv[])
{
    Toolkit::Import( KDLToolkit );
    
    Kuka160nAxesVelocityController my_robot("Robot");
    
    NAxesPositionViewer viewer("viewer");
    
    EmergencyStop _emergency(&my_robot);
    
    // Creating Event Handlers
    Handle _emergencyHandle = my_robot.events()->setupConnection("driveOutOfRange").
        callback(&_emergency,&EmergencyStop::callback).handle();
    Handle _positionWarning = my_robot.events()->setupConnection("positionOutOfRange").
        callback(&PositionLimitCallBack).handle();

    // Connecting Event Handlers
    _emergencyHandle.connect();
    _positionWarning.connect();
  
    //KinematicsComponents
    KinematicFamily* kukakf = new Kuka160();
    
    std::cout<<kukakf<<std::endl;
    
    //CartesianComponents
    CartesianSensor sensor("CartesianSensor",kukakf);
    CartesianGeneratorPos generator("CartesianGenerator");
    CartesianControllerPosVel controller("CartesianController");
    CartesianEffectorVel effector("CartesianEffector",kukakf);
  
    //connecting sensor and effector to hardware
    my_robot.connectPeers(&sensor);
    my_robot.connectPeers(&effector);

    //connecting components to eachother
    sensor.connectPeers(&generator);
    sensor.connectPeers(&controller);
    sensor.connectPeers(&effector);
    controller.connectPeers(&generator);
    controller.connectPeers(&effector);
    viewer.connectPeers(&my_robot);
    
    //Reporting
    FileReporting reporter("Reporting");
    reporter.connectPeers(&sensor);
    reporter.connectPeers(&generator);
    reporter.connectPeers(&controller);
    reporter.connectPeers(&effector);  
    
    //Create supervising TaskContext
    GenericTaskContext super("CartesianTest");
    
    // Link components to supervisor
    super.connectPeers(&my_robot);
    super.connectPeers(&reporter);
    super.connectPeers(&sensor);    
    super.connectPeers(&generator); 
    super.connectPeers(&controller);
    super.connectPeers(&effector);
    super.connectPeers(&viewer);
    
    //
    //// Load programs in supervisor
    //super.loadProgram("cpf/program_calibrate_offsets.ops");
    super.loadProgram("program_moveto.ops");
    //
    //// Load StateMachine in supervisor
    super.loadStateMachine("states.osd");

    // Creating Tasks
    NonPreemptibleActivity _kukaTask(0.01, my_robot.engine() ); 
    NonPreemptibleActivity _sensorTask(0.01, sensor.engine() ); 
    NonPreemptibleActivity _generatorTask(0.01, generator.engine() ); 
    NonPreemptibleActivity _controllerTask(0.01, controller.engine() ); 
    NonPreemptibleActivity _effectorTask(0.01, effector.engine() ); 
    PeriodicActivity reportingTask(3,0.02,reporter.engine());
    PeriodicActivity superTask(1,0.1,super.engine());
    PeriodicActivity viewerTask(3,0.1,viewer.engine());
    
    TaskBrowser browser(&super);
    browser.setColorTheme( TaskBrowser::whitebg );
    
    superTask.start();
    _kukaTask.start();
    viewerTask.start();
    
    // Start the console reader.
    browser.loop();
    
    delete kukakf;
    
    return 0;
}
