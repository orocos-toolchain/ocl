#include "ReportingComponent.hpp"

#include <corelib/SlaveActivity.hpp>
#include <execution/TaskBrowser.hpp>
#include <execution/Ports.hpp>

using namespace std;
using namespace ORO_CoreLib;
using namespace ORO_Execution;
using namespace ORO_Components;

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
    SlaveActivity slave(0.01);
    ReportingComponent rc("Reporting");
    TestTaskContext gtc("MyPeer");
    TestTaskContext2 gtc2("MyPeer2");

    rc.addPeer( &gtc );
    rc.addPeer( &gtc2 );
    gtc.connectPeers( &gtc2 );

    TaskBrowser tb( &rc );

    slave.run( rc.engine() );

    tb.loop();

    return 0;
}

