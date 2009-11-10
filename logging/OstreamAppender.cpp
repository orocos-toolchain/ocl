#include "logging/OstreamAppender.hpp"
#include "ocl/ComponentLoader.hpp"

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
    // \todo use v2.0 data flow to trigger this when new data is available

    if (!log_port.ready()) return;      // no category connected to us
    
    OCL::logging::LoggingEvent   event;
    if (log_port.Pop(event))
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
