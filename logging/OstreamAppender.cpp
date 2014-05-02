#include "logging/OstreamAppender.hpp"
#include "ocl/Component.hpp"

#include <log4cpp/OstreamAppender.hh>

using namespace RTT;

namespace OCL {
namespace logging {

OstreamAppender::OstreamAppender(std::string name) :
    OCL::logging::Appender(name),
    maxEventsPerCycle_prop("MaxEventsPerCycle", "Maximum number of log events to pop per cycle",1),
    maxEventsPerCycle(1)
{
    properties()->addProperty(maxEventsPerCycle_prop);
}

OstreamAppender::~OstreamAppender()
{
}

bool OstreamAppender::configureHook()
{
    // verify valid limits
    int m = maxEventsPerCycle_prop.rvalue();
    if ((0 > m))
    {
        log(Error) << "Invalid maxEventsPerCycle value of "
                   << m << ". Value must be >= 0."
                   << endlog();
        return false;
    }
    maxEventsPerCycle = m;

    if (!appender)
        appender = new log4cpp::OstreamAppender(getName(), &std::cout);

    return configureLayout();
}

void OstreamAppender::updateHook()
{
	processEvents(1);
}

void OstreamAppender::cleanupHook()
{
    /* normally in log4cpp the category owns the appenders and deletes them
     itself, however we don't associate appenders and categories in the
     same manner. Hence, you have to manually manage appenders.
     */
    delete appender;
    appender = 0;
}

// namespaces
}
}

ORO_LIST_COMPONENT_TYPE(OCL::logging::OstreamAppender);
