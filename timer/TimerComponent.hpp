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
        : public RTT::TaskContext,
          public RTT::Timer
    {
    protected:
        RTT::Event <void(RTT::Timer::TimerId)> mtimeoutEvent;

        /**
         * This hook will check if a NonPeriodicActivity has been properly
         * setup.
         */
        bool configureHook();
        bool startHook();
        void updateHook();
        void stopHook();
        void cleanupHook();

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

        virtual void timeout(RTT::Timer::TimerId id);

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
