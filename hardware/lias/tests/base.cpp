#include <rtt/os/main.h>

#include <hardware/lias/BaseVelocityController.hpp>
#include <hardware/wrench/WrenchSensor.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <rtt/PeriodicActivity.hpp>

#include "TestTcpTaskContext.hpp"

using namespace RTT;
using namespace std;
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
        log(Info) << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<endlog();
    }

    BaseVelocityController a_task("ATask");
    TestTcpTaskContext b_task("BTask");
    WrenchSensor c_task(0.1,"CTask",0);

    a_task.connectPeers( &b_task);
    b_task.connectPeers( &c_task);


    PeriodicActivity periodicActivityA(RTT::OS::HighestPriority, 0.1, a_task.engine() );
    PeriodicActivity periodicActivityB(RTT::OS::HighestPriority, 0.1, b_task.engine() );
    PeriodicActivity periodicActivityC(RTT::OS::HighestPriority, 0.1, c_task.engine() );


    TaskBrowser browser( &b_task );
    b_task.scripting()->loadPrograms("cpf/base.ops");

    browser.loop();

    periodicActivityA.stop();
    periodicActivityB.stop();

    return 0;
}
