#include "logging/OstreamAppender.hpp"
#include "ocl/Component.hpp"

#include <log4cpp/OstreamAppender.hh>

namespace OCL {
namespace logging {

OstreamAppender::OstreamAppender(std::string name) :
		OCL::logging::Appender(name)
{
}

OstreamAppender::~OstreamAppender()
{
}

bool OstreamAppender::configureHook()
{
    appender = new log4cpp::OstreamAppender(getName(), &std::cout);

    return configureLayout();
}

void OstreamAppender::updateHook()
{
    if (!log_port.connected()) return;      // no category connected to us

    OCL::logging::LoggingEvent   event;
    if (log_port.read( event ) == RTT::NewData)
    {
        log4cpp::LoggingEvent   e2 = event.toLog4cpp();
        assert(appender);
        appender->doAppend(e2);
    }
}

void OstreamAppender::cleanupHook()
{
    delete appender;
    appender = 0;
}

// namespaces
}
}

ORO_LIST_COMPONENT_TYPE(OCL::logging::OstreamAppender);
