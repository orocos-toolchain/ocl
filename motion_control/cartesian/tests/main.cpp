//hardware interfaces
#include <hardware/kuka/Kuka160nAxesVelocityController.hpp>
#include <hardware/kuka/Kuka361nAxesVelocityController.hpp>
#include <hardware/kuka/EmergencyStop.hpp>

//User interface
#include <taskbrowser/TaskBrowser.hpp>

//Reporting
#include <reporting/FileReporting.hpp>

//Cartesian components
#include <motion_control/cartesian/CartesianComponents.hpp>

//Kinematics component
//#include <kdl/bindings/rtt/toolkit.hpp>
#include <kdl/chain.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/os/main.h>
#include <rtt/Activities.hpp>

using namespace Orocos;
using namespace RTT;
using namespace KDL;
using namespace std;

// main() function

int ORO_main(int argc, char* argv[])
{
//    Toolkit::Import( KDLToolkit );

    Kuka361nAxesVelocityController my_robot("Robot");

    //CartesianComponents
    CartesianVelocityController cartesianrobot("CartesianRobot");
    CartesianGeneratorPos generator("CartesianGenerator");
    CartesianControllerPosVel controller("CartesianController");

    //connecting ports
    connectPorts(&cartesianrobot,&my_robot);
    connectPeers(&cartesianrobot,&my_robot);
    connectPorts(&cartesianrobot,&generator);
    connectPorts(&cartesianrobot,&controller);
    connectPorts(&generator,&controller);

    //Reporting
    FileReporting reporter("Reporting");
    reporter.connectPeers(&cartesianrobot);
    reporter.connectPeers(&generator);
    reporter.connectPeers(&controller);

    //Create supervising TaskContext
    TaskContext super("CartesianTest");

    // Link components to supervisor
    super.connectPeers(&my_robot);
    super.connectPeers(&reporter);
    super.connectPeers(&cartesianrobot);
    super.connectPeers(&generator);
    super.connectPeers(&controller);

    //
    //// Load programs in supervisor
    //super.loadProgram("cpf/program_calibrate_offsets.ops");
    super.scripting()->loadPrograms("cpf/program_moveto.ops");
    //
    //// Load StateMachine in supervisor
    super.scripting()->loadStateMachines("cpf/states.osd");

    // Creating Tasks
    PeriodicActivity robotTask(0,0.01, my_robot.engine() );
    PeriodicActivity cartesianrobotTask(0,0.01, cartesianrobot.engine() );
    PeriodicActivity generatorTask(0,0.01, generator.engine() );
    PeriodicActivity controllerTask(0,0.01, controller.engine() );
    PeriodicActivity reportingTask(3,0.02,reporter.engine());
    PeriodicActivity superTask(0,0.01,super.engine());

    TaskBrowser browser(&super);
    browser.setColorTheme( TaskBrowser::whitebg );

    superTask.start();

    // Start the console reader.
    browser.loop();

    superTask.stop();

    return 0;
}
