//hardware interfaces

#include <hardware/staubli/StaubliRX130nAxesVelocityController.hpp>
#include <hardware/kuka/EmergencyStop.hpp>


//User interface
#include <taskbrowser/TaskBrowser.hpp>

//Reporting
#include <reporting/FileReporting.hpp>

#include <rtt/os/main.h>
#include <rtt/PeriodicActivity.hpp>

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
    

    StaubliRX130nAxesVelocityController my_robot("Robot");
    
    EmergencyStop _emergency(&my_robot);
    
    /// Creating Event Handlers
    _emergency.addEvent(&my_robot,"driveOutOfRange");
    _emergency.addEvent(&my_robot,"positionOutOfRange");
    
    /// Link my_robot to Taskbrowser
    TaskBrowser browser(&my_robot );
    browser.setColorTheme( TaskBrowser::whitebg );
    
    /// Creating Tasks
    PeriodicActivity robotTask(0,0.002, my_robot.engine() );  
    
    /// Start the console reader.
    my_robot.configure();
    
    browser.loop();
    
    return 0;
}
