//#include <execution/GenericTaskContext.hpp>
#include <corelib/NonPreemptibleActivity.hpp>
//#include <corelib/Attribute.hpp>
//#include <corelib/Event.hpp>
//#include <execution/TemplateFactories.hpp>
//#include <execution/TaskBrowser.hpp>
//#include <execution/MethodC.hpp>
//#include <execution/CommandC.hpp>
//#include <execution/EventC.hpp>
//#include <execution/ConnectionC.hpp>
#include <execution/Ports.hpp>

#include "WrenchSensor.hpp"
#include <os/main.h>

//User interface
#include "../../taskbrowser/TaskBrowser.hpp"

//Reporting
#include "../../reporting/FileReporting.hpp"



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
    

    //CommandC init = a_task.commands()->create("this", "initialise").arg(arg1).arg(arg2).arg(arg3);

    NonPreemptibleActivity periodicActivityA(0.1, a_task.engine() );
    FileReporting reporter("Reporting");
    reporter.connectPeers(&a_task);
 
//    periodicActivityA.start(); 
//    periodicActivityB.start(); 


/*
    
    CommandC init = a_task.commands()->create("this", "initialise").arg(arg1).arg(arg2).arg(arg3);
    bool result;

    result = init.execute();
    if (result == false) 
    {
      Logger::log() << Logger::Error << "Initialise command failed."<<Logger::endl;
      return -1;
    }

     while ( !init.evaluate() )
     sleep(1);
                                                 
*/
    
    
    TaskBrowser browser( &a_task );

    browser.loop();

    periodicActivityA.stop();
    
    return 0;
}
