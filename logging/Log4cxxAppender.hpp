#ifndef	socketAppender_HPP
#define	socketAppender_HPP 1

#include <rtt/TaskContext.hpp>
#include <rtt/Property.hpp>
#include <rtt/InputPort.hpp>
#include "LoggingEvent.hpp"
#include <log4cxx/logger.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/net/socketappender.h>
#include <log4cxx/simplelayout.h>

#if 0
int main() {
   log4cxx::Log4cxxAppender * socketAppender = new
log4cxx::Log4cxxAppender(log4cxx::LayoutPtr(new log4cxx::SimpleLayout()),
"logfile", false);

   log4cxx::helpers::Pool p;
   socketAppender->activateOptions(p);

   log4cxx::BasicConfigurator::configure(log4cxx::AppenderPtr(socketAppender));
   log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getDebug());
   log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("logger");

   LOG4CXX_INFO(logger,"Created Log4cxxAppender appender");

   return 0;
}
#endif

namespace OCL {
namespace logging {

/**
 * Interoperability component which translates our log4cpp events to
 * log4cxx events and sends them to a network/socket appender.
 */
class Log4cxxAppender
    : public RTT::TaskContext
{
public:
	Log4cxxAppender(std::string name);
	virtual ~Log4cxxAppender();
protected:
    virtual bool configureHook();
	virtual void updateHook();
	virtual void cleanupHook();

	log4cxx::helpers::Pool p;
	log4cxx::net::SocketAppender * socketAppender;
	log4cxx::helpers::InetAddressPtr address;

    /// Port we receive logging events on
    /// Initially unconnected. The logging service connects appenders.
    RTT::InputPort<OCL::logging::LoggingEvent> log_port;

	   /// Name of host to append to
    std::string      hostname_prop;

    /// The port where the logging server runs.
    int port_prop;
    /**
     * Property to set maximum number of log events to pop per cycle
     */
    int              maxEventsPerCycle_prop;

    /**
     * Maximum number of log events to pop per cycle
     *
     * Defaults to 0.
     *
     * A value of 0 indicates to not limit the number of events per cycle.
     * With enough event production, this could lead to thread
     * starvation!
     */
    int                           maxEventsPerCycle;
};

// namespaces
}
}

#endif
