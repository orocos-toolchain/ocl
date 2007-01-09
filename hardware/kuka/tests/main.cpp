//hardware interfaces

#include <hardware/kuka/Kuka160nAxesVelocityController.hpp>
#include <hardware/kuka/Kuka361nAxesVelocityController.hpp>
#include <hardware/kuka/EmergencyStop.hpp>


//User interface
#include <taskbrowser/TaskBrowser.hpp>

//Reporting
#include <reporting/FileReporting.hpp>

#include <rtt/os/main.h>

using namespace Orocos;
using namespace RTT;
using namespace std;

/// main() function

int ORO_main(int argc, char* argv[])
{
    
    if ( Logger::log().getLogLevel() < Logger::Warning ) {
        Logger::log().setLogLevel( Logger::Warning );
        log(Info) << argv[0] << " manually raises LogLevel to 'Debug'."
                  << " See also file 'orocos.log'." << endlog();
    }
    

    TaskContext* my_robot = NULL;
    if (argc > 1)
        {
            string s = argv[1];
            if(s == "Kuka361"){
                log(Warning) <<"Choosing Kuka361"<<endlog();
                my_robot = new Kuka361nAxesVelocityController("Kuka361");
            }
            else if(s == "Kuka160"){
                log(Warning) <<"Choosing Kuka160"<<endlog();
                my_robot = new Kuka160nAxesVelocityController("Kuka160");
            }
        }
    else{
        log(Warning) <<"Using Default Kuka361"<<endlog();
        my_robot = new Kuka361nAxesVelocityController("Kuka361");
  }
    log(Info)<<"Robot Created"<<endlog();
    
    EmergencyStop _emergency(my_robot);
    
    /// Creating Event Handlers
    _emergency.addEvent(my_robot,"driveOutOfRange");
    _emergency.addEvent(my_robot,"positionOutOfRange");
    
    /// Link my_robot to Taskbrowser
    TaskBrowser browser(my_robot );
    browser.setColorTheme( TaskBrowser::whitebg );
    
    //Loading program in browser
    my_robot->scripting()->loadPrograms("cpf/program.ops");
    
    /// Creating Tasks
    PeriodicActivity _kukaTask(0,0.01, my_robot->engine() );  
    
    /// Start the console reader.
    _kukaTask.start();
    
    browser.loop();
    
    return 0;
}
