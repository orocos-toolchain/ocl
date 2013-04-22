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
    if (!log_port.connected()) return;      // no category connected to us

    /* Consume waiting events until
       a) the buffer is empty
       b) we consume too many events on one cycle
     */
    OCL::logging::LoggingEvent   event;
    assert(appender);
    assert(0 <= maxEventsPerCycle);
    if (0 == maxEventsPerCycle)
    {
        // consume infinite events
        for (;;)
        {
            if (log_port.read( event ) == NewData)
            {
                log4cpp::LoggingEvent   e2 = event.toLog4cpp();
                appender->doAppend(e2);
            }
            else
            {
                break;      // nothing to do
            }
        }
    }
    else
    {

        // consume up to maxEventsPerCycle events
        int n       = maxEventsPerCycle;
        do
        {
            if (log_port.read( event ) == NewData)
            {
                log4cpp::LoggingEvent   e2 = event.toLog4cpp();
                appender->doAppend(e2);
            }
            else
            {
                break;      // nothing to do
            }
            --n;
        }
        while (0 < n);
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
