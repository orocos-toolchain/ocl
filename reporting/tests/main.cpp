#include <rtt/os/main.h>
#include <reporting/ConsoleReporting.hpp>
#include <taskbrowser/TaskBrowser.hpp>

#include <rtt/extras/SlaveActivity.hpp>
#include <rtt/Activity.hpp>
#include <rtt/Port.hpp>

using namespace std;
using namespace Orocos;
using namespace RTT;

class TestTaskContext
    : public TaskContext
{
    Property<string> hello;
    OutputPort<std::vector<double> > dwport;
    InputPort<double> drport;
    Attribute<double> input;

public:
    TestTaskContext(std::string name)
        : TaskContext(name),
          hello("Hello", "The hello thing", "World"),
          dwport("D2Port"),
          drport("D1Port"),
          input("input")
    {
        this->properties()->addProperty( & hello );
        this->ports()->addPort( &drport );
        this->ports()->addPort( &dwport );
        this->attributes()->addAttribute( &input );

        // write initial value.
        std::vector<double> init(10, 1.0);
        dwport.setDataSample( init );
    }
};

class TestTaskContext2
    : public TaskContext
{
    Property<string> hello;
    OutputPort<double> dwport;
    InputPort<std::vector<double> > drport;
    Attribute<std::vector<double> > input;

public:
    TestTaskContext2(std::string name)
        : TaskContext(name),
          hello("Hello", "The hello thing", "World"),
          dwport("D1Port"),
          drport("D2Port"),
          input("input")
    {
        this->properties()->addProperty( & hello );
        this->ports()->addPort( &drport );
        this->ports()->addPort( &dwport );
        this->attributes()->addAttribute( &input );
    }
};

int ORO_main( int argc, char** argv)
{
    // Set log level more verbose than default,
    // such that we can see output :
    if ( Logger::log().getLogLevel() < Logger::Info ) {
        Logger::log().setLogLevel( Logger::Info );
        log(Info) << argv[0]
		      << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<endlog();
    }

    // Reporter's activity: not real-time !
    Activity act(ORO_SCHED_OTHER, 0, 1.0);
    ConsoleReporting rc("Reporting");
    TestTaskContext gtc("MyPeer");
    TestTaskContext2 gtc2("MyPeer2");

    TestTaskContext2 gtc3("MySoloPeer");

    rc.addPeer( &gtc );
    rc.addPeer( &gtc2 );
    rc.addPeer( &gtc3 );
    gtc.connectPorts( &gtc2 );

    TaskBrowser tb( &rc );

    act.run( rc.engine() );

    rc.marshalling()->loadProperties("reporter.cpf");
    rc.configure();

    cout <<endl<< "  This demo allows reporting of Components." << endl;
    cout << "  Use 'reportComponent(\"MyPeer\")' and/or 'reportComponent(\"MyPeer2\")'" <<endl;
    cout << "  Then invoke 'start()' and 'stop()'" <<endl;
    cout << "  Other methods (type 'this') are available as well."<<endl;

    tb.loop();

    act.stop();

    return 0;
}

