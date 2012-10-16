
#include "TimerComponent.hpp"
#include <rtt/Logger.hpp>
#include "ocl/Component.hpp"

ORO_CREATE_COMPONENT_TYPE()
ORO_LIST_COMPONENT_TYPE( OCL::TimerComponent )

namespace OCL
{
    using namespace std;
    using namespace RTT;

    TimerComponent::TimerComponent( std::string name /*= "os::Timer" */ )
        : TaskContext( name, PreOperational ), port_timers(32), mtimeoutEvent("timeout"),
          mtimer( port_timers, mtimeoutEvent ),
          waitForCommand( "waitFor", &TimerComponent::waitFor, this), //, &TimerComponent::isTimerExpired, this),
          waitCommand( "wait", &TimerComponent::wait, this) //&TimerComponent::isTimerExpired, this)
    {

        // Add the methods, methods make sure that they are
        // executed in the context of the (non realtime) caller.

        this->addOperation("arm", &os::Timer::arm , &mtimer, RTT::ClientThread).doc("Arm a single shot timer.").arg("timerId", "A numeric id of the timer to arm.").arg("delay", "The delay in seconds before it fires.");
        this->addOperation("startTimer", &os::Timer::startTimer , &mtimer, RTT::ClientThread).doc("Start a periodic timer.").arg("timerId", "A numeric id of the timer to start.").arg("period", "The period in seconds.");
        this->addOperation("killTimer", &os::Timer::killTimer , &mtimer, RTT::ClientThread).doc("Kill (disable) an armed or started timer.").arg("timerId", "A numeric id of the timer to kill.");
        this->addOperation("isArmed", &os::Timer::isArmed , &mtimer, RTT::ClientThread).doc("Check if a given timer is armed or started.").arg("timerId", "A numeric id of the timer to check.");
        this->addOperation("setMaxTimers", &os::Timer::setMaxTimers , &mtimer, RTT::ClientThread).doc("Raise or lower the maximum amount of timers.").arg("timers", "The largest amount of timers. The highest timerId is max-1.");
        this->addOperation( waitForCommand ).doc("Wait until a timer expires.").arg("timerId", "A numeric id of the timer to wait for.");
        this->addOperation( waitCommand ).doc("Arm and wait until that timer expires.").arg("timerId", "A numeric id of the timer to arm and to wait for.").arg("delay", "The delay in seconds before the timer expires.");
        this->addPort(mtimeoutEvent).doc("This port is written each time ANY timer expires. The timer id is the value sent in this port. This port is for backwards compatibility only. It is advised to use the timer_* ports.");
        for(unsigned int i=0;i<port_timers.size();i++){
            ostringstream port_name;
            port_name<<"timer_"<<i;
            port_timers[i] = new RTT::OutputPort<RTT::os::Timer::TimerId>(port_name.str());
            this->addPort(*(port_timers[i])).doc(string("This port is written each time ")+port_name.str()+string(" expires. The timer id is the value sent in this port."));
        }
    }

    TimerComponent::~TimerComponent() {
        this->stop();
        for(unsigned int i=0;i<port_timers.size();i++)
            delete port_timers[i];
    }

    bool TimerComponent::startHook()
    {
        return mtimer.getThread() && mtimer.getThread()->start();
    }

    void TimerComponent::updateHook()
    {
        // nop, we just process the wait commands.
    }

    void TimerComponent::stopHook()
    {
        mtimer.getThread()->stop();
    }

    bool TimerComponent::wait(RTT::os::Timer::TimerId id, double seconds)
    {
        return mtimer.arm(id, seconds);
    }

    bool TimerComponent::waitFor(RTT::os::Timer::TimerId id)
    {
        return true;
    }

    bool TimerComponent::isTimerExpired(RTT::os::Timer::TimerId id) const
    {
        return !mtimer.isArmed(id);
    }
}
