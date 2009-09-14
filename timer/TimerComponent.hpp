#ifndef ORO_TIMER_COMPONENT_HPP
#define ORO_TIMER_COMPONENT_HPP


#include <rtt/Event.hpp>
#include <rtt/TimeService.hpp>
#include <rtt/Command.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Timer.hpp>

#include <rtt/RTT.hpp>
#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * @brief A Component interface to the Real-Time Toolkit's timer.
     * It must be configured with a NonPeriodicActivity which will emit
     * the timeout event of this component.
     *
     */
    class TimerComponent
        : public RTT::TaskContext
    {
    protected:
        /**
         * Helper class for catching the virtual timeout function of Timer.
         */
        struct TimeoutCatcher : public Timer {
            RTT::Event <void(RTT::Timer::TimerId)>& me;

            TimeoutCatcher(RTT::Timer::TimerId max_timers, RTT::Event <void(RTT::Timer::TimerId)>& e) :
                    Timer(max_timers, ORO_SCHED_RT, OS::HighestPriority), me(e) 
            {}
            virtual void timeout(Timer::TimerId id) {
                me(id);
            }
        };

        TimeoutCatcher mtimer;
        RTT::Event <void(RTT::Timer::TimerId)> mtimeoutEvent;

        /**
         * This hook will check if a NonPeriodicActivity has been properly
         * setup.
         */
        bool startHook();
        void updateHook();
        void stopHook();

        /**
         * Command: wait until a timer expires.
         */
        RTT::Command<bool(RTT::Timer::TimerId)> waitForCommand;

        /**
         * Command: arm and wait until a timer expires.
         */
        RTT::Command<bool(RTT::Timer::TimerId, double)> waitCommand;

        /**
         * Command Implementation: wait until a timer expires.
         */
        bool waitFor(RTT::Timer::TimerId id);

        /**
         * Command Implementation: \b arm and wait until a timer expires.
         */
        bool wait(RTT::Timer::TimerId id, double seconds);

        /**
         * Command Condition: return true if \a id expired.
         */
        bool isTimerExpired(RTT::Timer::TimerId id) const;
    public:
        /**
         * Set up a component for timing events.
         */
        TimerComponent( std::string name = "Timer" );

        virtual ~TimerComponent();
    };

}

#endif
