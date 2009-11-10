#ifndef	LOGGINGSERVICE_HPP
#define	LOGGINGSERVICE_HPP 1

#include <rtt/TaskContext.hpp>
#include <rtt/PropertyBag.hpp>
#include <rtt/Method.hpp>

namespace OCL {
namespace logging {

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
    // list of appenders per category
    RTT::Property<RTT::PropertyBag>     appenders_prop;
    /** Log all categories
     * \warning Not realtime!
     */
    RTT::Method<void(void)>             logCategories_mtd;
    void logCategories();
};

// namespaces
}
}

#endif
