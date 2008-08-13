#include <boost/bind.hpp>
#include <timer/TimerComponent.hpp>
#include <taskbrowser/TaskBrowser.hpp>

#include <rtt/NonPeriodicActivity.hpp>
#include <rtt/PeriodicActivity.hpp>
#include <iostream>
#include <rtt/os/main.h>

using namespace std;
using namespace Orocos;
using namespace RTT;

class TestTaskContext
    : public TaskContext
{
    Handle h;
public:
    TestTaskContext(std::string name)
        : TaskContext(name, PreOperational)
    {
    }

    bool configureHook()
    {
        log(Info) << this->getName() <<" starts listening for timeout events." << endlog();
        if (this->getPeer("Timer") )
            h = this->getPeer("Timer")->events()->setupConnection("timeout").callback(this, &TestTaskContext::callback).handle();

        h.connect();
        return h.ready();
    }

    void callback(Timer::TimerId id)
    {
        log(Info) << this->getName() <<" detects timeout for timer " << id << endlog();
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


    TimerComponent tcomp("Timer");
    NonPeriodicActivity act(ORO_SCHED_RT, OS::HighestPriority, tcomp.engine() );

    TestTaskContext gtc("Peer");
    PeriodicActivity p_act(ORO_SCHED_RT, OS::HighestPriority, 0.1, gtc.engine() );

    gtc.addPeer(&tcomp);

    TaskBrowser tb( &gtc );

    gtc.configure();
    gtc.start();
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

