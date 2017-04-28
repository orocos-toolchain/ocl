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
       - method to log to a category
    */

    // Correspond to log4cpp::Priority::PriorityLevel
    RTT::Attribute<int>         level_EMERG_attr;
    RTT::Attribute<int>         level_FATAL_attr;
    RTT::Attribute<int>         level_ALERT_attr;
    RTT::Attribute<int>         level_CRIT_attr;
    RTT::Attribute<int>         level_ERROR_attr;
    RTT::Attribute<int>         level_WARN_attr;
    RTT::Attribute<int>         level_NOTICE_attr;
    RTT::Attribute<int>         level_INFO_attr;
    RTT::Attribute<int>         level_DEBUG_attr;
    RTT::Attribute<int>         level_NOTSET_attr;

    /// @copydoc OCL::logging::LoggingService::setCategoryPriority()
    RTT::Operation<bool(std::string,int)>       setCategoryPriority_mtd;
    /** Set the priority of category \a name to \a priority
     * If the category does not exist then an error is logged and nothing
     * else occurs.
     * @param name Category name
     * @param priority Priority level (one of level_XXX_attr)
     * @return true if category \a name exists and the category was set,
     * otherwise false
     */
    bool setCategoryPriority(const std::string& name, const int priority);

    /// @copydoc OCL::logging::LoggingService::getCategoryPriorityName()
    RTT::Operation<std::string(std::string)>    getCategoryPriorityName_mtd;
    /** Get the priority name of category \a name
     * @return if category \a name exists and the priority of the category is
     * known then the descriptive name (e.g. "INFO") of the priority, otherwise
     * if the category \a name exists but the priority is not known then
     * "UNKNOWN PRIORITY", otherwise "UNKNOWN CATEGORY"
     */
    std::string getCategoryPriorityName(const std::string& name);

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
