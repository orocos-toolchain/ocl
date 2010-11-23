#include <boost/bind.hpp>
#include <timer/TimerComponent.hpp>
#include <taskbrowser/TaskBrowser.hpp>

#include <rtt/Activity.hpp>
#include <rtt/InputPort.hpp>
#include <iostream>
#include <rtt/os/main.h>

using namespace std;
using namespace Orocos;
using namespace RTT;

class TestTaskContext
    : public RTT::TaskContext
{
    InputPort<os::Timer::TimerId> receiver;
public:
    TestTaskContext(std::string name)
        : RTT::TaskContext(name, PreOperational),
          receiver("TimerIn")
    {
        ports()->addEventPort( receiver );
    }

    bool configureHook()
    {
        if ( receiver.connected() )
            log(Info) << this->getName() <<" starts listening for timeout events." << endlog();
        return receiver.connected();
    }

    void updateHook()
    {
        os::Timer::TimerId id;
        if (receiver.read(id) == NewData)
            log(Info) << this->getName() <<" detects timeout for timer " << id << endlog();
    }
};

int ORO_main( int argc, char** argv)
{
    // Set log level more verbose than default,
    // such that we can see output :
    if ( RTT::Logger::log().getLogLevel() < RTT::Logger::Info ) {
        RTT::Logger::log().setLogLevel( RTT::Logger::Info );
        log(Info) << argv[0]
		      << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<endlog();
    }


    TimerComponent tcomp("Timer");
    tcomp.setActivity( new RTT::Activity(ORO_SCHED_RT, os::HighestPriority, 0.0) );

    TestTaskContext gtc("Peer");
    gtc.setActivity( new RTT::Activity(ORO_SCHED_RT, os::HighestPriority, 0.1) );

    gtc.ports()->getPort("TimerIn")->connectTo( tcomp.ports()->getPort("timeout"));

    TaskBrowser tb( &gtc );

    gtc.configure();
    gtc.start();
    gtc.addPeer( &tcomp );
    tcomp.configure();
    tcomp.start();

    cout <<endl<< "  This demo allows testing the TimerComponent." << endl;
    cout << "  Use 'Timer.arm(0, 1.5)' to arm timer '0' to end over 1.5 seconds. " <<endl;
    cout << "  32 timers are initially available (0..31)." <<endl;
    cout << "  Other methods (type 'this') are available as well."<<endl;

    tb.loop();

    tcomp.stop();
    gtc.stop();

    return 0;
}

