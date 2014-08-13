#ifndef ORO_TIMER_COMPONENT_HPP
#define ORO_TIMER_COMPONENT_HPP


#include <rtt/os/TimeService.hpp>
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
            std::vector<RTT::OutputPort<RTT::os::Timer::TimerId>* >& m_port_timers;
            TimeoutCatcher(std::vector<RTT::OutputPort<RTT::os::Timer::TimerId>* >& port_timers, RTT::OutputPort<RTT::os::Timer::TimerId>&  op) :
                os::Timer(port_timers.size(), ORO_SCHED_RT, os::HighestPriority),
                me(op),
		m_port_timers(port_timers)
            {}
            virtual void timeout(os::Timer::TimerId id) {
                m_port_timers[id]->write(id);
                me.write(id);
            }
        };

        std::vector<OutputPort<RTT::os::Timer::TimerId>* > port_timers;
        OutputPort<RTT::os::Timer::TimerId> mtimeoutEvent;
        TimeoutCatcher mtimer;

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
