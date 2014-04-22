#include "logging/GenerationalFileAppender.hpp"
#include "ocl/Component.hpp"
#include <rtt/Logger.hpp>

#include <log4cpp/GenerationalFileAppender.hh>

using namespace RTT;

namespace OCL {
namespace logging {

GenerationalFileAppender::GenerationalFileAppender(std::string name) :
    OCL::logging::Appender(name),
    advanceGeneration_op("advanceGeneration", &GenerationalFileAppender::advanceGeneration, this, RTT::OwnThread),
    filename_prop("Filename", "Name of file to log to"),
    maxEventsPerCycle_prop("MaxEventsPerCycle",
                           "Maximum number of log events to pop per cycle",
                           1),
    maxEventsPerCycle(1)
{
	provides()->addOperation(advanceGeneration_op).doc("Advance to the next logfile generation");

    properties()->addProperty(filename_prop);
    properties()->addProperty(maxEventsPerCycle_prop);
}

GenerationalFileAppender::~GenerationalFileAppender()
{
}

bool GenerationalFileAppender::configureHook()
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

    // \todo error checking

    appender = new log4cpp::GenerationalFileAppender(getName(),
													 filename_prop.get());

    return configureLayout();
}

void GenerationalFileAppender::updateHook()
{
	processEvents(maxEventsPerCycle);
}

void GenerationalFileAppender::cleanupHook()
{
    /* normally in log4cpp the category owns the appenders and deletes them
       itself, however we don't associate appenders and categories in the
       same manner. Hence, you have to manually manage appenders.
    */
    delete appender;
    appender = 0;
}

void GenerationalFileAppender::advanceGeneration()
{
	if (0 != appender)
	{
		static_cast<log4cpp::GenerationalFileAppender*>(appender)->advanceGeneration();
	}
	else
	{
		log(Error) << "No appender to roll over!" << endlog();
	}
}

// namespaces
}
}

ORO_LIST_COMPONENT_TYPE(OCL::logging::GenerationalFileAppender);
