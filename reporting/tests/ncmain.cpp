#include <iostream>

#include <rtt/os/main.h>
#include <reporting/NetcdfReporting.hpp>
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
    OutputPort<char> cwport;
    OutputPort<short> swport;
    OutputPort<int> iwport;
    OutputPort<float> fwport;
    OutputPort<std::vector<double> > dwport;
    InputPort<char> crport;
    InputPort<short> srport;
    InputPort<int> irport;
    InputPort<float> frport;
    InputPort<double> drport;  
    int i;
public:
    TestTaskContext(std::string name)
        : TaskContext(name),
          hello("Hello", "The hello thing", "World"),
          cwport("cw_port", 'a'),
          swport("sw_port", 1),
          iwport("iw_port", 0),
          fwport("fw_port", 0.0),
          dwport("dw_port"),
          crport("cr_port"),
          srport("sr_port"),
          irport("ir_port"),
          frport("fr_port"),
          drport("dr_port"), i(0)
    { 
        this->properties()->addProperty( hello );
        this->ports()->addPort( cwport );
        this->ports()->addPort( swport );
        this->ports()->addPort( iwport );
        this->ports()->addPort( fwport );
        this->ports()->addPort( dwport );
        this->ports()->addPort( crport );
        this->ports()->addPort( srport );
        this->ports()->addPort( irport );
        this->ports()->addPort( frport );
        this->ports()->addPort( drport );
        // write initial value.
        std::vector<double> init(10, 5.4528);
        dwport.write( init );
    }

    void updateHook() {
    	cwport.write(i);
    	swport.write(i);
    	iwport.write(i);
    	fwport.write(i);
    	dwport.write( std::vector<double>(10,i) );
    	++i;
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


    NetcdfReporting rc("Reporting");
    rc.setPeriod(0.01);
    TestTaskContext gtc("MyPeer");
    gtc.setPeriod(0.005);
    TestTaskContext gtc2("MyPeer2");
    gtc.setPeriod(0.02);

    TestTaskContext gtc3("MyPeer3");
    gtc.setPeriod(0.01);

    rc.addPeer( &gtc );
    rc.addPeer( &gtc2 );
    rc.addPeer( &gtc3 );
    gtc.connectPeers( &gtc2 );

    TaskBrowser tb( &rc );

    cout <<endl<< "  This demo allows reporting of 3 Components at 100Hz, 200Hz and 50Hz." << endl;
    cout << "  Use 'reportComponent(\"MyPeer\")' and/or 'reportComponent(\"MyPeer2\")'" <<endl;
    cout << "  Then invoke 'start()' and 'stop()'" <<endl;
    cout << "  Other methods (type 'this') are available as well."<<endl;
        
    tb.loop();

    return 0;
}

