#include "logging/Category.hpp"
#include <rtt/Logger.hpp>
#include <log4cpp/NDC.hh>
#include <log4cpp/HierarchyMaintainer.hh>

using namespace RTT;

namespace OCL {
namespace logging {

static const int BUFFER_SIZE = 20;  // \todo

Category::Category(const std::string& name,
                   log4cpp::Category* parent,
                   log4cpp::Priority::Value priority) :
        log4cpp::Category(name, parent, priority),
        log_port(convertName(name), BUFFER_SIZE)
{
}

Category::~Category()
{
}

void Category::log(log4cpp::Priority::Value priority,
                 const OCL::String& message) throw()
{
    if (isPriorityEnabled(priority))
    {
        _logUnconditionally2(priority, message);
    }
}

void Category::debug(const OCL::String& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::DEBUG))
        _logUnconditionally2(log4cpp::Priority::DEBUG, message);
}

void Category::info(const OCL::String& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::INFO))
        _logUnconditionally2(log4cpp::Priority::INFO, message);
}

void Category::notice(const OCL::String& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::NOTICE))
        _logUnconditionally2(log4cpp::Priority::NOTICE, message);
}

void Category::warn(const OCL::String& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::WARN))
        _logUnconditionally2(log4cpp::Priority::WARN, message);
}

void Category::error(const OCL::String& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::ERROR))
        _logUnconditionally2(log4cpp::Priority::ERROR, message);
}

void Category::crit(const OCL::String& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::CRIT))
        _logUnconditionally2(log4cpp::Priority::CRIT, message);
}

void Category::alert(const OCL::String& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::ALERT))
        _logUnconditionally2(log4cpp::Priority::ALERT, message);
}

void Category::emerg(const OCL::String& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::EMERG))
        _logUnconditionally2(log4cpp::Priority::EMERG, message);
}

void Category::fatal(const OCL::String& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::FATAL))
        _logUnconditionally2(log4cpp::Priority::FATAL, message);
}


void Category::_logUnconditionally2(log4cpp::Priority::Value priority,
                                    const OCL::String& message) throw()
{
    try
    {
        OCL::logging::LoggingEvent event(OCL::String(getName().c_str()),
                                         OCL::String(message.c_str()),
                                         // NDC's are not real-time
//                                     OCL::String(log4cpp::NDC::get().c_str()),
                                         OCL::String(""),
                                         priority);
        callAppenders(event);
    }
    catch (std::bad_alloc& e)
    {
        // \todo do what?
//        std::cerr << "Failed to log event: out of real-time memory" << std::endl;
        /// \todo ++numFailedLog
    }
}

void Category::callAppenders(const OCL::logging::LoggingEvent& event) throw()
{
    log_port.write( event );

    // let our parent categories append (if they want to)
    if (getAdditivity() && (getParent() != NULL))
    {
        OCL::logging::Category* parent = dynamic_cast<OCL::logging::Category*>(getParent());
        if (parent)
        {
            parent->callAppenders(event);
        }
        // else we don't use non-realtime parent Category's!
    }
}

std::string Category::convertName(const std::string& name)
{
    std::string     rc(name);

    std::replace_if(rc.begin(),
                    rc.end(),
                    std::bind2nd(std::equal_to<char>(), '.'),
                    '_');

    return rc;
}

log4cpp::Category* Category::createOCLCategory(const std::string& name,
                                               log4cpp::Category* parent,
                                               log4cpp::Priority::Value priority)
{
    // do _NOT_ log from within this function! You will cause a lockup due to
    // recursive calls to log4cpp, if you use RTT w/ log4cpp support.

    // \todo try catch on memory exceptions or failures?
    OCL::logging::Category* c = new OCL::logging::Category(name, parent, priority);
    return c;
}

// namespaces
}
}
