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

protected:
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
};

// namespaces
}
}

#endif
