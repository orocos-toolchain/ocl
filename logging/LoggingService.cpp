#include "logging/LoggingService.hpp"
#include "logging/Category.hpp"
#include "ocl/Component.hpp"

#include <boost/algorithm/string.hpp>
#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/HierarchyMaintainer.hh>

#include <rtt/Logger.hpp>
#include <typeinfo>

using namespace RTT;
using namespace std;

namespace OCL {
namespace logging {

LoggingService::LoggingService(std::string name) :
		RTT::TaskContext(name),
        levels_prop("Levels","A PropertyBag defining the level of each category of interest."),
        additivity_prop("Additivity","A PropertyBag defining the additivity of each category of interest."),
        appenders_prop("Appenders","A PropertyBag defining the appenders for each category of interest."),
        level_EMERG_attr ("EMERG",  log4cpp::Priority::EMERG ),
        level_FATAL_attr ("FATAL",  log4cpp::Priority::FATAL ),
        level_ALERT_attr ("ALERT",  log4cpp::Priority::ALERT ),
        level_CRIT_attr  ("CRIT",   log4cpp::Priority::CRIT  ),
        level_ERROR_attr ("ERROR",  log4cpp::Priority::ERROR ),
        level_WARN_attr  ("WARN",   log4cpp::Priority::WARN  ),
        level_NOTICE_attr("NOTICE", log4cpp::Priority::NOTICE),
        level_INFO_attr  ("INFO",   log4cpp::Priority::INFO  ),
        level_DEBUG_attr ("DEBUG",  log4cpp::Priority::DEBUG ),
        level_NOTSET_attr("NOTSET", log4cpp::Priority::NOTSET),
        setCategoryPriority_mtd("setCategoryPriority", &LoggingService::setCategoryPriority, this),
        getCategoryPriorityName_mtd("getCategoryPriorityName", &LoggingService::getCategoryPriorityName, this),
        logCategories_mtd("logCategories", &LoggingService::logCategories, this)
{
    this->properties()->addProperty( levels_prop );
    this->properties()->addProperty( additivity_prop );
    this->properties()->addProperty( appenders_prop );
    this->provides()->addAttribute(level_EMERG_attr);
    this->provides()->addAttribute(level_FATAL_attr);
    this->provides()->addAttribute(level_ALERT_attr);
    this->provides()->addAttribute(level_CRIT_attr);
    this->provides()->addAttribute(level_ERROR_attr);
    this->provides()->addAttribute(level_WARN_attr);
    this->provides()->addAttribute(level_NOTICE_attr);
    this->provides()->addAttribute(level_INFO_attr);
    this->provides()->addAttribute(level_DEBUG_attr);
    this->provides()->addAttribute(level_NOTSET_attr);
    this->provides()->addOperation( setCategoryPriority_mtd ).doc("Set the priority of category to p").arg("name", "Name of the category").arg("p", "Priority to set");
    this->provides()->addOperation( getCategoryPriorityName_mtd ).doc("Get the priority name of category").arg("name", "Name of the category");
    this->provides()->addOperation( logCategories_mtd ).doc("Log category hierarchy (not realtime!)");
}

LoggingService::~LoggingService()
{
}

bool LoggingService::configureHook()
{
    log(Debug) << "Configuring LoggingService" << endlog();

    // set the priority/level for each category

    PropertyBag bag = levels_prop.value();  // an empty bag is ok

    bool ok = true;
    PropertyBag::const_iterator it;
    for (it=bag.getProperties().begin(); it != bag.getProperties().end(); ++it)
    {
        Property<std::string>* category = dynamic_cast<Property<std::string>* >( *it );
        if ( !category )
        {
            log(Error) << "Expected Property '"
                       << (*it)->getName() << "' to be of type string." << endlog();
        }
        else 
        {
            std::string categoryName = category->getName();
            std::string levelName    = category->value();

            // "" == categoryName implies the root category.

            // \todo else if level is empty
            
            // log4cpp only takes upper case
            boost::algorithm::to_upper(levelName);
            
            log4cpp::Priority::Value priority = log4cpp::Priority::NOTSET;
            try
            {
                priority = log4cpp::Priority::getPriorityValue(levelName);
                
            }
            catch (std::invalid_argument)
            {
                // \todo more descriptive
                log(Error) << "Bad level name: " << levelName << endlog();
                return false;
            }
            
            log(Debug) << "Getting category '" << categoryName << "'" << endlog();
            // will create category if not exists
            log4cpp::Category& category =
                log4cpp::Category::getInstance(categoryName);

            category.setPriority(priority);
            log(Info) << "Category '" << categoryName 
                      << "' has priority '" << levelName << "'"
                      << endlog();
        }
    }

    // first clear all existing appenders in order to avoid double connects:
    for(vector<string>::iterator it = active_appenders.begin(); it != active_appenders.end(); ++it) {
        base::PortInterface* port = 0;
        TaskContext* appender	= getPeer(*it);
        if (appender && (port = appender->ports()->getPort("LogPort")) )
            port->disconnect();
    }
    if ( !active_appenders.empty() ) 
        log(Warning) <<"Reconfiguring LoggingService '"<<getName() << "': I've removed all existing Appender connections and will now rebuild them."<<endlog();
    active_appenders.clear();

	// set the additivity of each category

    bag = additivity_prop.value();  // an empty bag is ok

    for (it=bag.getProperties().begin(); it != bag.getProperties().end(); ++it)
    {
        Property<bool>* category = dynamic_cast<Property<bool>* >( *it );
        if ( !category )
        {
            log(Error) << "Expected Property '"
                       << (*it)->getName() << "' to be of type boolean." << endlog();
        }
        else
        {
            std::string categoryName	= category->getName();
            bool		additivity		= category->value();

            // "" == categoryName implies the root category.

            log(Debug) << "Getting category '" << categoryName << "'" << endlog();
            // will create category if not exists
            log4cpp::Category& category =
            log4cpp::Category::getInstance(categoryName);

            category.setAdditivity(additivity);
            log(Info) << "Category '" << categoryName
                      << "' has additivity '" << std::string(additivity ? "on":"off") << "'"
                      << endlog();
        }
    }

    // create a port for each appender, and associate category/appender

    bag = appenders_prop.value();           // an empty bag is ok

    ok = true;
    for (it=bag.getProperties().begin(); it != bag.getProperties().end(); ++it)
    {
        Property<std::string>* association = dynamic_cast<Property<std::string>* >( *it );
        if ( !association )
        {
            log(Error) << "Expected Property '"
                       << (*it)->getName() << "' to be of type string." << endlog();
        }
        // \todo else if name or level are empty
        else 
        {
            std::string categoryName    = association->getName();
            std::string appenderName    = association->value();
            
            // find category - will create category if not exists
            log4cpp::Category& p   = log4cpp::Category::getInstance(categoryName);
            OCL::logging::Category* category =
                dynamic_cast<OCL::logging::Category*>(&p);
            if (0 == category)
            {
                log(Error) << "Category '" << categoryName << "' is not an OCL category: type is '" << typeid(p).name() << "'" << endlog();
                ok = false;
                break;
            }
            
            // find appender
            RTT::TaskContext* appender	= getPeer(appenderName);
            if (appender)
            {
                // connect category port with appender port
                RTT::base::PortInterface* appenderPort = 0;

                appenderPort	= appender->ports()->getPort("LogPort");
                if (appenderPort)
                {
                    // \todo make connection policy configurable (from xml).
                    ConnPolicy cp = ConnPolicy::buffer(100,ConnPolicy::LOCK_FREE,false,false);
                    if ( appenderPort->connectTo( &(category->log_port), cp) )
                    {
                        std::stringstream   str;
                        str << "Category '" << categoryName
                            << "' has appender '" << appenderName << "'" 
                            << " with level "
                            << log4cpp::Priority::getPriorityName(category->getPriority());
                        log(Info) << str.str() << endlog();
//                        std::cout << str.str() << std::endl;
                        active_appenders.push_back(appenderName);
                    }
                    else
                    {
                        log(Error) << "Failed to connect port to appender '" << appenderName << "'" << endlog();
                        ok = false;
                        break;
                    }
                }
                else
                {
                    log(Error) << "Failed to find log port in appender" << endlog();
                    ok = false;
                    break;
                }
            }
            else
            {
                log(Error) << "Could not find appender '" << appenderName << "'" << endlog();
                ok = false;
                break;
            }
        }
    }
    
    return ok;
}

bool LoggingService::setCategoryPriority(const std::string& name,
                                         const int          priority)
{
    bool rc = false;        // prove otherwise
    log4cpp::Category* c = log4cpp::Category::exists(name);
    if (NULL != c)
    {
        try {
            c->setPriority(priority);
            const std::string level = log4cpp::Priority::getPriorityName(priority);
            rc = true;
            log(Info)  << "Category '" << name << "' set to priority '" << level << "'"  << endlog();
        } catch (...) {
            log(Error) << "Priority value '" << priority << "' is not known!" << endlog();
        }
    }
    else
    {
        log(Error) << "Could not find category '" << name << "'" << endlog();
    }
    return rc;
}

std::string LoggingService::getCategoryPriorityName(const std::string& name)
{
    std::string rc;
    log4cpp::Category* c = log4cpp::Category::exists(name);
    if (NULL != c)
    {
        try {
            rc = log4cpp::Priority::getPriorityName(c->getPriority());
            log(Info)  << "Category '" << name << "' has priority '" << rc << "'" << endlog();
        } catch (...) {
            rc = "UNKNOWN PRIORITY";
            log(Error) << "Category '" << name << "' has unknown priority!" << endlog();
        }
    }
    else
    {
        rc = "UNKNOWN CATEGORY";
        log(Error) << "Could not find category '" << name << "'" << endlog();
    }
    return rc;
}

// NOT realtime
void LoggingService::logCategories()
{
    std::vector<log4cpp::Category*>*            categories =
        log4cpp::Category::getCurrentCategories();
    assert(categories);
    std::vector<log4cpp::Category*>::iterator   iter;
    log(Info) << "Number categories = " << (int)categories->size() << endlog();
    for (iter = categories->begin(); iter != categories->end(); ++iter)
    {
        std::stringstream str;

        OCL::logging::Category* c = dynamic_cast<OCL::logging::Category*>(*iter);
        str
            << "Category '" << (*iter)->getName() << "', level="
            << log4cpp::Priority::getPriorityName((*iter)->getPriority())
            << ", typeid='"
            << typeid(*iter).name()
            << "', type really is '" 
            << std::string(0 != c ? "OCL::Category" : "log4cpp::Category")
            << "', additivity=" << (const char*)((*iter)->getAdditivity()?"yes":"no");
        if (0 != c)
        {
            str << ", port=" << (c->log_port.connected() ? "connected" : "not connected");
        }
        log4cpp::Category* p = (*iter)->getParent();
        if (p)
        {
            str << ", parent name='" << p->getName() << "'";
        }
        else
        {
            str << ", No parent";
        }

        log(Info) << str.str() << endlog();
    }
}
   
// namespaces
}
}

ORO_CREATE_COMPONENT_TYPE();
ORO_LIST_COMPONENT_TYPE(OCL::logging::LoggingService);
