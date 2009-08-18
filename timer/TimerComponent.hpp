#ifndef ORO_TIMER_COMPONENT_HPP
#define ORO_TIMER_COMPONENT_HPP


#include <rtt/Event.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Command.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/os/Timer.hpp>

#include <rtt/RTT.hpp>
#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * @brief A Component interface to the Real-Time types::Toolkit's timer.
     * It must be configured with a RTT::Activity which will emit
     * the timeout event of this component.
     *
     */
    class TimerComponent
        : public RTT::TaskContext,
          public RTT::os::Timer
    {
    protected:
        RTT::Event <void(RTT::os::Timer::TimerId)> mtimeoutEvent;

        /**
         * This hook will check if a RTT::Activity has been properly
         * setup.
         */
        bool configureHook();
        bool startHook();
        void updateHook();
        void stopHook();
        void cleanupHook();

        /**
         * RTT::Command: wait until a timer expires.
         */
        RTT::Command<bool(RTT::os::Timer::TimerId)> waitForCommand;

        /**
         * RTT::Command: arm and wait until a timer expires.
         */
        RTT::Command<bool(RTT::os::Timer::TimerId, double)> waitCommand;

        /**
         * RTT::Command Implementation: wait until a timer expires.
         */
        bool waitFor(RTT::os::Timer::TimerId id);

        /**
         * RTT::Command Implementation: \b arm and wait until a timer expires.
         */
        bool wait(RTT::os::Timer::TimerId id, double seconds);

        /**
         * RTT::Command Condition: return true if \a id expired.
         */
        bool isTimerExpired(RTT::os::Timer::TimerId id) const;
    public:
        /**
         * Set up a component for timing events.
         */
        TimerComponent( std::string name = "os::Timer" );

        virtual ~TimerComponent();

        virtual void timeout(RTT::os::Timer::TimerId id);

        virtual bool stop();

        /**
         * Returns the current system time in seconds.
         */
        double getTime() const;

        /**
         * Returns the time elapsed since a given system time.
         */
        double secondsSince(double stamp) const;

    };

}

#endif
