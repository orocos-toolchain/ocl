#include "logging/Category.hpp"
#include <rtt/Logger.hpp>
#include <log4cpp/NDC.hh>
#include <log4cpp/HierarchyMaintainer.hh>

using namespace RTT;

namespace OCL {
namespace logging {

Category::Category(const std::string& name,
                   log4cpp::Category* parent,
                   log4cpp::Priority::Value priority) :
        log4cpp::Category(name, parent, priority),
        log_port( convertName(name) )
{
}

Category::~Category()
{
}

void Category::log(log4cpp::Priority::Value priority,
                 const RTT::rt_string& message) throw()
{
    if (isPriorityEnabled(priority))
    {
        _logUnconditionally2(priority, message);
    }
}

void Category::debug(const RTT::rt_string& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::DEBUG))
        _logUnconditionally2(log4cpp::Priority::DEBUG, message);
}

void Category::info(const RTT::rt_string& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::INFO))
        _logUnconditionally2(log4cpp::Priority::INFO, message);
}

void Category::notice(const RTT::rt_string& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::NOTICE))
        _logUnconditionally2(log4cpp::Priority::NOTICE, message);
}

void Category::warn(const RTT::rt_string& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::WARN))
        _logUnconditionally2(log4cpp::Priority::WARN, message);
}

void Category::error(const RTT::rt_string& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::ERROR))
        _logUnconditionally2(log4cpp::Priority::ERROR, message);
}

void Category::crit(const RTT::rt_string& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::CRIT))
        _logUnconditionally2(log4cpp::Priority::CRIT, message);
}

void Category::alert(const RTT::rt_string& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::ALERT))
        _logUnconditionally2(log4cpp::Priority::ALERT, message);
}

void Category::emerg(const RTT::rt_string& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::EMERG))
        _logUnconditionally2(log4cpp::Priority::EMERG, message);
}

void Category::fatal(const RTT::rt_string& message) throw()
{
    if (isPriorityEnabled(log4cpp::Priority::FATAL))
        _logUnconditionally2(log4cpp::Priority::FATAL, message);
}


void Category::_logUnconditionally2(log4cpp::Priority::Value priority,
                                    const RTT::rt_string& message) throw()
{
    try
    {
        OCL::logging::LoggingEvent event(RTT::rt_string(getName().c_str()),
                                         RTT::rt_string(message.c_str()),
                                         // NDC's are not real-time
//                                     RTT::rt_string(log4cpp::NDC::get().c_str()),
                                         RTT::rt_string(""),
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

CategoryStream Category::getRTStream(log4cpp::Priority::Value priority)
{
    return CategoryStream(this, isPriorityEnabled(priority) ? 
                          priority : log4cpp::Priority::NOTSET);
}

bool Category::connectToLogPort(RTT::base::PortInterface& otherPort)
{
    return otherPort.connectTo(&log_port);
}

// namespaces
}
}
