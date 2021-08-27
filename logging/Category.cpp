#include "logging/Category.hpp"
#include <rtt/Logger.hpp>
#include <rtt/ConnPolicy.hpp>
#include <log4cpp/NDC.hh>
#include <log4cpp/HierarchyMaintainer.hh>

using namespace RTT;

namespace OCL {
namespace logging {

Category::Category(const std::string& name,
                   log4cpp::Category* parent,
                   log4cpp::Priority::Value priority) :
        log4cpp::Category(name, parent, priority),
        log_port( convertName(name) , false )
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


void Category::_logUnconditionally2(log4cpp::Priority::Value priority,
                                    const RTT::rt_string& message) throw()
{
    try
    {
        OCL::logging::LoggingEvent event(getName(),
                                         message,
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
    // only write if the port is connected
    if (log_port.connected())
    {
        log_port.write( event );
    }

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

bool Category::connectToLogPort(RTT::base::PortInterface&   otherPort,
                                RTT::ConnPolicy&            cp)
{
    return otherPort.connectTo(&log_port, cp);
}

// namespaces
}
}
