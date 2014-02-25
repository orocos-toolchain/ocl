#include "logging/RollingFileAppender.hpp"
#include "ocl/Component.hpp"
#include <rtt/Logger.hpp>

#include <log4cpp/RollingFileAppender.hh>

using namespace RTT;

namespace OCL {
namespace logging {

RollingFileAppender::RollingFileAppender(std::string name) :
		OCL::logging::Appender(name), 
        filename_prop("Filename", "Name of file to log to"),
        maxFileSize_prop("MaxFileSize", 
						 "Maximum file size (in bytes) before rolling over",
						 10 * 1024 * 1024),	// default in log4cpp
        maxBackupIndex_prop("MaxBackupIndex", 
							"Maximum number of backup files to keep",
							1),				// default in log4cpp
        maxEventsPerCycle_prop("MaxEventsPerCycle", 
							   "Maximum number of log events to pop per cycle",
							   1),
        maxEventsPerCycle(1)
{
    properties()->addProperty(filename_prop);
    properties()->addProperty(maxEventsPerCycle_prop);
    properties()->addProperty(maxFileSize_prop);
    properties()->addProperty(maxBackupIndex_prop);
}

RollingFileAppender::~RollingFileAppender()
{
}

bool RollingFileAppender::configureHook()
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

	log(Info) << "maxfilesize " << maxFileSize_prop.get() 
			  << " maxbackupindex " << maxBackupIndex_prop.get() << std::endl;
    appender = new log4cpp::RollingFileAppender(getName(), 
												filename_prop.get(),
												maxFileSize_prop.get(),
												maxBackupIndex_prop.get());
    
    return configureLayout();
}

void RollingFileAppender::updateHook()
{
	processEvents(maxEventsPerCycle);
}

void RollingFileAppender::cleanupHook()
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

ORO_LIST_COMPONENT_TYPE(OCL::logging::RollingFileAppender);
