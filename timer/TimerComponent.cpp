
#include "TimerComponent.hpp"

#include <rtt/Method.hpp>
#include <rtt/Logger.hpp>

#include "ocl/ComponentLoader.hpp"

ORO_CREATE_COMPONENT( OCL::TimerComponent )

namespace OCL
{
    using namespace std;
    using namespace RTT;

    TimerComponent::TimerComponent( std::string name /*= "os::Timer" */ )
        : TaskContext( name, PreOperational ), mtimer( 32, mtimeoutEvent ),
          mtimeoutEvent("timeout"),
          waitForCommand( "waitFor", &TimerComponent::waitFor, &TimerComponent::isTimerExpired, this),
          waitCommand( "wait", &TimerComponent::wait, &TimerComponent::isTimerExpired, this)
    {

        // Add the methods, methods make sure that they are
        // executed in the context of the (non realtime) caller.

        this->methods()->addMethod( method( "arm", &os::Timer::arm , &mtimer),
                                    "Arm a single shot timer.",
                                    "timerId", "A numeric id of the timer to arm.",
                                    "delay", "The delay in seconds before it fires.");
        this->methods()->addMethod( method( "startTimer", &os::Timer::startTimer , &mtimer),
                                    "Start a periodic timer.",
                                    "timerId", "A numeric id of the timer to start.",
                                    "period", "The period in seconds.");
        this->methods()->addMethod( method( "killTimer", &os::Timer::killTimer , &mtimer),
                                    "Kill (disable) an armed or started timer.",
                                    "timerId", "A numeric id of the timer to kill.");
        this->methods()->addMethod( method( "isArmed", &os::Timer::isArmed , &mtimer),
                                    "Check if a given timer is armed or started.",
                                    "timerId", "A numeric id of the timer to check.");
        this->methods()->addMethod( method( "setMaxTimers", &os::Timer::setMaxTimers , &mtimer),
                                    "Raise or lower the maximum amount of timers.",
                                    "timers", "The largest amount of timers. The highest timerId is max-1.");
        this->events()->addEvent( &mtimeoutEvent,
                                  "Timeout is emitted each time a timer expires.",
                                  "timerId", "The numeric id of the timer which expired.");

        this->commands()->addCommand( &waitForCommand, "Wait until a timer expires.",
                                    "timerId", "A numeric id of the timer to wait for.");
        this->commands()->addCommand( &waitCommand, "Arm and wait until that timer expires.",
                                    "timerId", "A numeric id of the timer to arm and to wait for.",
                                    "delay", "The delay in seconds before the timer expires.");
    }

    TimerComponent::~TimerComponent() {
        this->stop();
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
