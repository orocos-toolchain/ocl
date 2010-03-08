#ifndef ORO_TIMER_COMPONENT_HPP
#define ORO_TIMER_COMPONENT_HPP


#include <rtt/os/TimeService.hpp>
#include <rtt/Method.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/os/Timer.hpp>
#include <rtt/OutputPort.hpp>

#include <rtt/RTT.hpp>
#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * @brief A Component interface to the Real-Time types::Toolkit's timer.
     * It must be configured with a Activity which will emit
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
        struct TimeoutCatcher : public os::Timer {
            RTT::OutputPort<RTT::os::Timer::TimerId>& me;
            TimeoutCatcher(RTT::os::Timer::TimerId max_timers, RTT::OutputPort<RTT::os::Timer::TimerId>& e) :
                    os::Timer(max_timers, ORO_SCHED_RT, os::HighestPriority), me(e)
            {}
            virtual void timeout(os::Timer::TimerId id) {
                me.write(id);
            }
        };

        TimeoutCatcher mtimer;
        OutputPort<RTT::os::Timer::TimerId> mtimeoutEvent;

        /**
         * This hook will check if a Activity has been properly
         * setup.
         */
        bool startHook();
        void updateHook();
        void stopHook();

        /**
         * Command: wait until a timer expires.
         */
        RTT::Operation<bool(RTT::os::Timer::TimerId)> waitForCommand;

        /**
         * Command: arm and wait until a timer expires.
         */
        RTT::Operation<bool(RTT::os::Timer::TimerId, double)> waitCommand;

        /**
         * Command Implementation: wait until a timer expires.
         */
        bool waitFor(RTT::os::Timer::TimerId id);

        /**
         * Command Implementation: \b arm and wait until a timer expires.
         */
        bool wait(RTT::os::Timer::TimerId id, double seconds);

        /**
         * Command Condition: return true if \a id expired.
         */
        bool isTimerExpired(RTT::os::Timer::TimerId id) const;
    public:
        /**
         * Set up a component for timing events.
         */
        TimerComponent( std::string name = "os::Timer" );

        virtual ~TimerComponent();
    };

}

#endif
