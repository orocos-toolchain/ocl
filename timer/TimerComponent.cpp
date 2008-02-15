
#include "TimerComponent.hpp"

#include <rtt/Method.hpp>
#include <rtt/Logger.hpp>

#include "ocl/ComponentLoader.hpp"

ORO_CREATE_COMPONENT( OCL::TimerComponent )

namespace OCL
{
    using namespace std;
    using namespace RTT;

    TimerComponent::TimerComponent( std::string name /*= "Timer" */ ) 
        : TaskContext( name, PreOperational ), Timer( 32 ),
          mtimeoutEvent("timeout"),
          waitForCommand( "waitFor", &TimerComponent::waitFor, &TimerComponent::isTimerExpired, this),
          waitCommand( "wait", &TimerComponent::wait, &TimerComponent::isTimerExpired, this)
    {

        // Add the methods, methods make sure that they are 
        // executed in the context of the (non realtime) caller.
        
        this->methods()->addMethod( method( "arm", &Timer::arm , this),
                                    "Arm a single shot timer.",
                                    "timerId", "A numeric id of the timer to arm.",
                                    "delay", "The delay in seconds before it fires.");
        this->methods()->addMethod( method( "startTimer", &Timer::startTimer , this),
                                    "Start a periodic timer.",
                                    "timerId", "A numeric id of the timer to start.",
                                    "period", "The period in seconds.");
        this->methods()->addMethod( method( "killTimer", &Timer::killTimer , this),
                                    "Kill (disable) an armed or started timer.",
                                    "timerId", "A numeric id of the timer to kill.");
        this->methods()->addMethod( method( "isArmed", &Timer::isActive , this),
                                    "Check if a given timer is armed or started.",
                                    "timerId", "A numeric id of the timer to check.");
        this->methods()->addMethod( method( "setMaxTimers", &Timer::setMaxTimers , this),
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
        this->cleanup();
    }

    void TimerComponent::timeout(Timer::TimerId id) {
        mtimeoutEvent(id);
    }

    bool TimerComponent::configureHook()
    {
        if (this->engine()->getActivity() == 0) {
            log(Error) << "You must assign a non periodic activity to this TimerComponent." <<endlog();
            return false;
        }

        if ( this->engine()->getActivity()->isPeriodic() ) {
            log(Error) << "Can not run TimerComponent with a periodic activity. Use a non periodic activity." <<endlog();
            return false;
        }
        
        log(Info) <<"TimerComponent correctly configured."<<endlog();
        Timer::setActivity( this->engine()->getActivity() );
        assert( this->getActivity() );
        return true;
    }

    void TimerComponent::cleanupHook()
    {
        Timer::setActivity( 0 );
    }        

    bool TimerComponent::startHook()
    {
        return Timer::initialize();
    }

    void TimerComponent::updateHook()
    {
        // ok,ok, we won't process events or commands ourselves...
        Timer::loop();
    }

    bool TimerComponent::stop()
    {
        return Timer::breakLoop() && TaskContext::stop();
    }

    void TimerComponent::stopHook()
    {
        Timer::finalize();
    }

    bool TimerComponent::wait(RTT::Timer::TimerId id, double seconds)
    {
        return this->arm(id, seconds);
    }

    bool TimerComponent::waitFor(RTT::Timer::TimerId id)
    {
        return true;
    }

    bool TimerComponent::isTimerExpired(RTT::Timer::TimerId id) const
    {
        return !Timer::isActive(id);
    }
}
