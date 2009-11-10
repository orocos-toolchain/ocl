#include "logging/FileAppender.hpp"
#include "ocl/ComponentLoader.hpp"
#include <rtt/Logger.hpp>

#include <log4cpp/FileAppender.hh>

using namespace RTT;

namespace OCL {
namespace logging {

FileAppender::FileAppender(std::string name) :
		OCL::logging::Appender(name), 
        filename_prop("Filename", "Name of file to log to")
{
    properties()->addProperty(&filename_prop);
}

FileAppender::~FileAppender()
{
}

bool FileAppender::configureHook()
{
    // \todo error checking

    appender = new log4cpp::FileAppender(getName(), filename_prop.rvalue());
    
    return configureLayout();
}

void FileAppender::updateHook()
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
    else
    {
//        log(Debug) << "FileAppender " << getName() << " found empty buffer." << endlog();
    }
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
