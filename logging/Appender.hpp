#ifndef	APPENDER_HPP
#define	APPENDER_HPP 1

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>
#include "LoggingEvent.hpp"

// forward declare
namespace log4cpp {
class Appender;
}
    
namespace OCL {
namespace logging {

class Appender : public RTT::TaskContext
{
public:
	Appender(std::string name);
	virtual ~Appender();

    /** Optionally create a layout according to \a layoutName and \a layoutPattern.
        \pre !appender, if you want a layout to actually be created
        \return true if not layout or no appender or a layout was created
        successfully, otherwise false
    */
    virtual bool configureLayout();
    /// ensure port is connected before we start
    virtual bool startHook();
	/// Drain the buffer
	virtual void stopHook();

	/** Process all remaining events in buffer
	 */
	virtual void drainBuffer();

protected:
	/** Process up \a n events
        @param n if 0 ==n then process events until buffer is empty, otherwise
        process at most n events
        @pre 0 <= n (otherwise acts as though n==1)
     */
	virtual void processEvents(int n);

    /// Port we receive logging events on
    /// Initially unconnected. The logging service connects appenders.
    RTT::InputPort<OCL::logging::LoggingEvent> log_port;

    /// Appender created by derived class
    log4cpp::Appender*                              appender;

    // support layouts for all appenders
    /// Layout name (e.g. "simple", "basic", "pattern")
    RTT::Property<std::string>                      layoutName_prop;
    /// Layout conversion pattern (for those layouts that use a pattern)
    RTT::Property<std::string>                      layoutPattern_prop;

	// diagnostic: count number of times popped max events
	unsigned int countMaxPopped;
};

// namespaces
}
}

#endif
