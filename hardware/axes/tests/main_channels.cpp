
#include <iostream>
#include <taskbrowser/TaskBrowser.hpp>
#include <hardware/axes/AxesComponent.hpp>
#include <hardware/axes/dev/SimulationAxis.hpp>

#include <rtt/os/main.h>
#include <rtt/PeriodicActivity.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Logger.hpp>

using namespace std;
using namespace Orocos;
using namespace RTT;


int ORO_main( int argc, char** argv)
{
    // Set log level more verbose than default,
    // such that we can see output :
    if ( Logger::log().getLogLevel() < Logger::Info ) {
        Logger::log().setLogLevel( Logger::Info );
        Logger::log() << Logger::Info << argv[0] 
		      << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<Logger::endl;
    }


    PeriodicActivity act(10, 1.0);
    AxesComponent ac(5,"Axes");

    SimulationAxis ax1;
    SimulationAxis ax2;
    SimulationAxis ax3;

    ac.addAxis("XAxis", &ax1);
    ac.addAxis("YAxis", &ax2);
    ac.addAxis("ZAxis", &ax3);

    ac.addAxisOnChannel("XAxis", "Position", 0);
    ac.addAxisOnChannel("YAxis", "Position", 1);
    ac.addAxisOnChannel("ZAxis", "Position", 2);

    TaskBrowser tb( &ac );

    act.run( ac.engine() );

    cout <<endl<< "  This demo allows to manipulate simulated axes." << endl;
    cout << "  Try the following commands (one by one):"<<endl;
    cout << "  leave, Axes.start(), Axes.enableAxis(\"XAxis\"),"<<endl;
    cout << "  Axes.testData[0] = 3.0, InputValues.Set(Axes.testData), OutputValues.Get(),"<<endl;
    cout << "  XAxis_Velocity.Get(), Axes.disableAxis(\"XAxis\"), XAxes_Velocity.Get(),..."<<endl;
    
    cout << "  Other methods (type 'this') are available as well."<<endl;
        
    tb.loop();

    act.stop();

    return 0;
}

