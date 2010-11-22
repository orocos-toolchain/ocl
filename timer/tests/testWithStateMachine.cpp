#include <string>

#include <ocl/HMIConsoleOutput.hpp>
#include <timer/TimerComponent.hpp>
#include <taskbrowser/TaskBrowser.hpp>

#include <rtt/scripting/Scripting.hpp>
#include <rtt/Activity.hpp>
#include <rtt/scripting/StateMachine.hpp>
#include <iostream>
#include <rtt/os/main.h>

using namespace std;
using namespace Orocos;
using namespace RTT;
using namespace boost;

// test TimerComponent when used by state machine (ie via Orocos interface)
class TestStateMachine
    : public TaskContext
{
    Handle h;
	// log a message
	RTT::Operation<void(std::string)>					log_mtd;
    InputPort<os::Timer::TimerId> receiver;

public:
    TestStateMachine(std::string name) :
            TaskContext(name, PreOperational),
            log_mtd("log", &TestStateMachine::doLog, this)
    {
        addOperation( log_mtd ).doc("Log a message").arg("message", "Message to log");
        addEventPort("TimerIn", receiver);
    }

    bool startHook()
    {
        bool 				rc = false;		// prove otherwise
        scripting::StateMachinePtr 	p;
        shared_ptr<Scripting> scripting = getProvider<Scripting>("scripting");
        if (!scripting)
            return false;
        Logger::In			in(getName());
        std::string         machineName = this->getName();
        if ( scripting->hasStateMachine(machineName))
        {
            if (scripting->activateStateMachine(machineName))
            {
                if (scripting->startStateMachine(machineName))
                {
                    rc = true;
                }
                else
                {
                    log(Error) << "Unable to start state machine: " << machineName << endlog();
                }
            }
            else
            {
                log(Error) << "Unable to activate state machine: " << machineName << endlog();
            }
        }
        else
        {
            log(Error) << "Unable to find state machine: " << machineName << endlog();
        }
        return rc;
    }

    void doLog(std::string message)
    {
        Logger::In			in(getName());
        log(Info) << message << endlog();
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

    HMIConsoleOutput hmi("hmi");
    hmi.setActivity( new Activity(ORO_SCHED_RT, os::HighestPriority, 0.1) );

    TimerComponent tcomp("Timer");
    tcomp.setActivity( new Activity(ORO_SCHED_RT, os::HighestPriority ) );

    TestStateMachine peer("testWithStateMachine");  // match filename
    peer.setActivity( new Activity(ORO_SCHED_RT, os::HighestPriority, 0.1 ) );

    peer.addPeer(&tcomp);
    peer.addPeer(&hmi);

    peer.ports()->getPort("TimerIn")->connectTo( tcomp.ports()->getPort("timeout"));

    std::string name = "testWithStateMachine.osd";
    assert (peer.getProvider<Scripting>("scripting"));
	if ( !peer.getProvider<Scripting>("scripting")->loadStateMachines(name) )
    {
        log(Error) << "Unable to load state machine: '" << name << "'" << endlog();
        tcomp.getActivity()->stop();
        return -1;
    }

    TaskBrowser tb( &peer );

    peer.configure();
    peer.start();
    tcomp.configure();
    tcomp.start();
    hmi.start();

    tb.loop();

    tcomp.stop();
    peer.stop();

    return 0;
}

