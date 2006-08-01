#include "taskbrowser/TaskBrowser.hpp"

#include <rtt/SlaveActivity.hpp>
#include <rtt/PeriodicActivity.hpp>
#include <rtt/Ports.hpp>
#include <rtt/os/main.h>

using namespace std;
using namespace Orocos;
using namespace RTT;

class TestTaskContext
    : public GenericTaskContext
{
    Property<string> hello;
    WriteDataPort<std::vector<double> > dwport;
    ReadDataPort<double> drport;

public:
    TestTaskContext(std::string name)
        : GenericTaskContext(name),
          hello("Hello", "The hello thing", "World"),
          dwport("D2Port"),
          drport("D1Port")
    {
        this->properties()->addProperty( &hello );
        this->ports()->addPort( &drport );
        this->ports()->addPort( &dwport );

        // write initial value.
        std::vector<double> init(10, 1.0);
        dwport.Set( init );
    }
};

class TestTaskContext2
    : public GenericTaskContext
{
    Property<string> hello;
    WriteDataPort<double> dwport;
    ReadDataPort<std::vector<double> > drport;

public:
    TestTaskContext2(std::string name)
        : GenericTaskContext(name),
          hello("Hello", "The hello thing", "World"),
          dwport("D1Port"),
          drport("D2Port")
    {
        this->properties()->addProperty( & hello );
        this->ports()->addPort( &drport );
        this->ports()->addPort( &dwport );
    }
};

int ORO_main( int argc, char** argv)
{
    // Set log level more verbose than default,
    // such that we can see output :
    if ( Logger::log().getLogLevel() < Logger::Info ) {
        Logger::log().setLogLevel( Logger::Info );
        Logger::log() << Logger::Info << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<Logger::endl;
    }

    PeriodicActivity act(10, 1.0);

    TestTaskContext gtc("MyPeer");
    TestTaskContext2 gtc2("MyPeer2");

    TestTaskContext2 gtc3("MySoloPeer");

    gtc.connectPeers( &gtc2 );

    TaskBrowser tb( &gtc );

    Logger::log()<<Logger::Info <<Logger::endl<< "  This demo demonstrates interaction with Components." << Logger::endl;
    Logger::log()<<Logger::Info << "  Use 'enter' and/or 'leave' to go 'inside' or 'outside' a component. " <<Logger::endl;
    Logger::log()<<Logger::Info << "  The inside interface shows the methods and ports of the visited component," <<Logger::endl;
    Logger::log()<<Logger::Info << "  the outside interface show the methods and ports of the TaskBrowser."<<Logger::endl;
        
    tb.loop();

    act.stop();

    return 0;
}

