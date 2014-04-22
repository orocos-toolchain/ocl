#ifndef	LOGGINGSERVICE_HPP
#define	LOGGINGSERVICE_HPP 1

#include <rtt/TaskContext.hpp>
#include <rtt/PropertyBag.hpp>
#include <rtt/Operation.hpp>

namespace OCL {
namespace logging {

/**
 * This component is responsible for reading the logging configuration
 * setting up the logging categories and connecting to the appenders.
 * You may have multiple LoggingService components, but each appender
 * may only belong to one LoggingService. Usually, you'll only have
 * one LoggingService per application.
 *
\* Adding an Appender to the LoggingService is done with the addPeer()
 * method of the TaskContext class, ie loggingservice->addPeer(fileappender)
 *
 * @see http://www.orocos.org/wiki/rtt/examples-and-tutorials/using-real-time-logging
 */
class LoggingService : public RTT::TaskContext
{
public:
	LoggingService(std::string name);
	virtual ~LoggingService();
    
    virtual bool configureHook();

    /* \todo

       configuration of this component
       - specify category for this component to log to
       - specify priority of logging of this component

       services for other components
       - method to set priority of any category
       - method to get priority of any category
       - method to log to a category
    */

protected:
    // list of all category levels
    RTT::Property<RTT::PropertyBag>     levels_prop;
    // list of all category additivity values (0 == false == additivity off)
    RTT::Property<RTT::PropertyBag>     additivity_prop;
    // list of appenders per category
    RTT::Property<RTT::PropertyBag>     appenders_prop;
    // list of all active appenders
    std::vector<std::string>            active_appenders;
    /** Log all categories
     * \warning Not realtime!
     */
    RTT::Operation<void(void)>             logCategories_mtd;
    void logCategories();
};

// namespaces
}
}

#endif
