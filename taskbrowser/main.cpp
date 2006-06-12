#include "TaskBrowser.hpp"

#include <corelib/SlaveActivity.hpp>
#include <corelib/PeriodicActivity.hpp>
#include <execution/Ports.hpp>
#include <os/main.h>

using namespace std;
using namespace Orocos;
using namespace RTT;

class TestTaskContext
    : public GenericTaskContext
{
    Property<string> hello;
    ReadDataPort<double> drport;
    WriteDataPort<std::vector<double> > dwport;
public:
    TestTaskContext(std::string name)
        : GenericTaskContext(name),
          hello("Hello", "The hello thing", "World"),
          dwport("D2Port"),
          drport("D1Port")
    {
        this->attributes()->addProperty( & hello );
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
    ReadDataPort<std::vector<double> > drport;
    WriteDataPort<double> dwport;
public:
    TestTaskContext2(std::string name)
        : GenericTaskContext(name),
          hello("Hello", "The hello thing", "World"),
          dwport("D1Port"),
          drport("D2Port")
    {
        this->attributes()->addProperty( & hello );
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

    cout <<endl<< "  This demo allows reporting of Components." << endl;
    cout << "  Use 'reportComponent(\"MyPeer\")' and/or 'reportComponent(\"MyPeer2\")'" <<endl;
    cout << "  Then invoke 'start()' and 'stop()'" <<endl;
    cout << "  Other methods (type 'this') are available as well."<<endl;
        
    tb.loop();

    act.stop();

    return 0;
}

