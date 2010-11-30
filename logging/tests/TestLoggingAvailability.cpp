#include "logging/tests/TestLoggingAvailability.hpp"
#include <rtt/rt_string.hpp>
#include "logging/Category.hpp"

#include <rtt/Logger.hpp>
#include "ocl/Component.hpp"

#include <log4cpp/HierarchyMaintainer.hh>

namespace OCL {
namespace logging {
namespace test {

static const char* categoryName = "org.orocos.ocl.logging.tests";

LoggingAvailability::LoggingAvailability(std::string name) :
		RTT::TaskContext(name),
        logger(dynamic_cast<OCL::logging::Category*>(
                   &log4cpp::Category::getInstance(
                       categoryName + std::string(".") + name)))
{
}

LoggingAvailability::~LoggingAvailability()
{
}

bool LoggingAvailability::configureHook()
{
    if (logger)
    {
        logger->info(RTT::rt_string("Available in configureHook"));
    }
    else
    {
        log(Error) << "Not available in configureHook()" << endlog();
    }
    return true;
}    

bool LoggingAvailability::startHook()
{
    if (logger)
    {
        logger->info(RTT::rt_string("Available in startHook"));
    }
    else
    {
        log(Error) << "Not available in startHook()" << endlog();
    }
    return true;
}
        
void LoggingAvailability::updateHook()
{
    if (logger)
    {
        logger->info(RTT::rt_string("Available in updateHook"));
    }
    else
    {
        log(Error) << "Not available in updateHook()" << endlog();
    }
}

void LoggingAvailability::stopHook()
{
    if (logger)
    {
        logger->info(RTT::rt_string("Available in stopHook"));
    }
    else
    {
        log(Error) << "Not available in stopHook()" << endlog();
    }
}

// namespaces
}
}
}

//ORO_CREATE_COMPONENT_TYPE();
ORO_LIST_COMPONENT_TYPE(OCL::logging::test::LoggingAvailability);
