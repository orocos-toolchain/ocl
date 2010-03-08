#include "taskbrowser/TaskBrowser.hpp"

#include <rtt/extras/SlaveActivity.hpp>
#include <rtt/Activity.hpp>
#include <rtt/Port.hpp>
#include <rtt/os/main.h>

using namespace std;
using namespace Orocos;
using namespace RTT;

class TestTaskContext
    : public RTT::TaskContext
{
    RTT::Property<string> hello;
    OutputPort<std::vector<double> > dwport;
    InputPort<double> drport;

public:
    TestTaskContext(std::string name)
        : RTT::TaskContext(name),
          hello("Hello", "The hello thing", "World"),
          dwport("D2Port"),
          drport("D1Port")
    {
        this->properties()->addProperty( hello );
        this->ports()->addPort( drport );
        this->ports()->addPort( dwport );

        std::vector<double> init(10, 1.0);
        dwport.write(init);
    }
};

class TestTaskContext2
    : public RTT::TaskContext
{
    RTT::Property<string> hello;
    OutputPort<double> dwport;
    InputPort<std::vector<double> > drport;

public:
    TestTaskContext2(std::string name)
        : RTT::TaskContext(name),
          hello("Hello", "The hello thing", "World"),
          dwport("D1Port"),
          drport("D2Port")
    {
        this->properties()->addProperty( hello );
        this->ports()->addPort( drport );
        this->ports()->addPort( dwport );

        // write initial value.
        dwport.write( 0.0 );
    }
};

int ORO_main( int argc, char** argv)
{
    // Set log level more verbose than default,
    // such that we can see output :
    if ( RTT::Logger::log().getLogLevel() < RTT::Logger::Info ) {
        RTT::Logger::log().setLogLevel( RTT::Logger::Info );
        log(Info) << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<endlog();
    }

    TestTaskContext gtc("MyPeer");
    TestTaskContext2 gtc2("MyPeer2");

    TestTaskContext2 gtc3("MySoloPeer");

    gtc.connectPeers( &gtc2 );

    TaskBrowser tb( &gtc );

    log(Info) <<endlog()<< "  This demo demonstrates interaction with Components." << endlog();
    log(Info) << "  Use 'enter' and/or 'leave' to go 'inside' or 'outside' a component. " <<endlog();
    log(Info) << "  The inside interface shows the methods and ports of the visited component," <<endlog();
    log(Info) << "  the outside interface show the methods and ports of the TaskBrowser."<<endlog();

    RTT::Activity act(10, 1.0, gtc.engine());

    tb.loop();

    act.stop();

    return 0;
}

