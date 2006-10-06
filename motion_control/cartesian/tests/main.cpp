//hardware interfaces
#include <hardware/kuka/Kuka160nAxesVelocityController.hpp>
#include <hardware/kuka/Kuka361nAxesVelocityController.hpp>
#include <hardware/kuka/EmergencyStop.hpp>

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
#include <kdl/kinfam/kuka361.hpp>
#include <kdl/toolkit.hpp>
#include <kdl/kinfam/kinematicfamily_io.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/os/main.h>

using namespace Orocos;
using namespace RTT;
using namespace KDL;
using namespace std;

// main() function

int ORO_main(int argc, char* argv[])
{
    Toolkit::Import( KDLToolkit );

    TaskContext* my_robot = NULL;
    KinematicFamily* kukakf = NULL;
    if (argc > 1)
        {
            string s = argv[1];
            if(s == "Kuka361"){
                Logger::log()<<Logger::Warning<<"Choosing Kuka361"<<Logger::endl;
                my_robot = new Kuka361nAxesVelocityController("Robot");
                kukakf = new Kuka361();
            }
            else if(s == "Kuka160"){
                Logger::log()<<Logger::Warning<<"Choosing Kuka160"<<Logger::endl;
                my_robot = new Kuka160nAxesVelocityController("Robot");
                kukakf = new Kuka160();
            }
        }
    else{
        Logger::log()<<Logger::Warning<<"Using Default Kuka160"<<Logger::endl;
        my_robot = new Kuka160nAxesVelocityController("Robot");
        kukakf = new Kuka160();
    }
    
    NAxesPositionViewer viewer("viewer");
    
    EmergencyStop _emergency(my_robot);
    
    /// Creating Event Handlers
    _emergency.addEvent(my_robot,"driveOutOfRange");
    //_emergency.addEvent(&my_robot,"positionOutOfRange");

  
    //KinematicsComponents
        
    //CartesianComponents
    CartesianSensor sensor("CartesianSensor",kukakf);
    CartesianGeneratorPos generator("CartesianGenerator");
    CartesianControllerPosVel controller("CartesianController");
    CartesianEffectorVel effector("CartesianEffector",kukakf);
  
    //connecting sensor and effector to hardware
    my_robot->connectPorts(&sensor);
    my_robot->connectPorts(&effector);

    //connecting components to eachother
    sensor.connectPorts(&generator);
    sensor.connectPorts(&controller);
    sensor.connectPorts(&effector);
    controller.connectPorts(&generator);
    controller.connectPorts(&effector);
    viewer.connectPorts(my_robot);
    
    //Reporting
    FileReporting reporter("Reporting");
    reporter.connectPeers(&sensor);
    reporter.connectPeers(&generator);
    reporter.connectPeers(&controller);
    reporter.connectPeers(&effector);  
    
    //Create supervising TaskContext
    TaskContext super("CartesianTest");
    
    // Link components to supervisor
    super.connectPeers(my_robot);
    super.connectPeers(&reporter);
    super.connectPeers(&sensor);    
    super.connectPeers(&generator); 
    super.connectPeers(&controller);
    super.connectPeers(&effector);
    super.connectPeers(&viewer);
    
    //
    //// Load programs in supervisor
    //super.loadProgram("cpf/program_calibrate_offsets.ops");
    super.scripting()->loadPrograms("cpf/program_moveto.ops");
    //
    //// Load StateMachine in supervisor
    super.scripting()->loadStateMachines("cpf/states.osd");

    // Creating Tasks
    NonPreemptibleActivity _kukaTask(0.01, my_robot->engine() ); 
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

    viewerTask.stop();
    _kukaTask.stop();
    superTask.stop();
        
    delete kukakf;
    delete my_robot;
   
    return 0;
}
