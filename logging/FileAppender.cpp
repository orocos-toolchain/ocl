#include "logging/FileAppender.hpp"
#include "ocl/Component.hpp"
#include <rtt/Logger.hpp>

#include <log4cpp/FileAppender.hh>

using namespace RTT;

namespace OCL {
namespace logging {

FileAppender::FileAppender(std::string name) :
		OCL::logging::Appender(name),
        filename_prop("Filename", "Name of file to log to"),
        maxEventsPerCycle_prop("MaxEventsPerCycle", "Maximum number of log events to pop per cycle",1),
        maxEventsPerCycle(1)
{
    properties()->addProperty(filename_prop);
    properties()->addProperty(maxEventsPerCycle_prop);
}

FileAppender::~FileAppender()
{
}

bool FileAppender::configureHook()
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
    if (appender)
        delete appender; // in case the filename changed...

    appender = new log4cpp::FileAppender(getName(), filename_prop.rvalue());

    return configureLayout();
}

void FileAppender::updateHook()
{
	processEvents(maxEventsPerCycle);
}

void FileAppender::cleanupHook()
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

ORO_LIST_COMPONENT_TYPE(OCL::logging::FileAppender);
