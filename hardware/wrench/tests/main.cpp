#include <rtt/NonPreemptibleActivity.hpp>
#include <rtt/Ports.hpp>

#include "hardware/wrench/WrenchSensor.hpp"
#include <rtt/os/main.h>

//User interface
#include "taskbrowser/TaskBrowser.hpp"

//Reporting
#include "reporting/FileReporting.hpp"



using namespace std;
using namespace RTT;
using namespace Orocos;

/**
 * main() function
 */
int ORO_main(int arc, char* argv[])
{
    // Set log level more verbose than default,
    // such that we can see output :
    if ( Logger::log().getLogLevel() < Logger::Info ) {
        Logger::log().setLogLevel( Logger::Info );
        Logger::log() << Logger::Info << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<Logger::endl;
    }

    WrenchSensor a_task(0.1,"ATask",0);
    
    NonPreemptibleActivity periodicActivityA(0.1, a_task.engine() );
    FileReporting reporter("Reporting");
    reporter.connectPeers(&a_task);
 
    TaskBrowser browser( &a_task );

    browser.loop();

    periodicActivityA.stop();
    
    return 0;
}
