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
        logCategories_mtd("logCategories", &LoggingService::logCategories, this)
{
    this->properties()->addProperty( levels_prop );
    this->properties()->addProperty( additivity_prop );
    this->properties()->addProperty( appenders_prop );
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
            
            // find category 
            log4cpp::Category* p = log4cpp::HierarchyMaintainer::getDefaultMaintainer().getExistingInstance(categoryName);
            OCL::logging::Category* category =
                dynamic_cast<OCL::logging::Category*>(p);
            if (0 == category)
            {
                if (0 != p)
                {
                    log(Error) << "Category '" << categoryName << "' is not an OCL category: type is '" << typeid(*p).name() << "'" << endlog();
                }
                else
                {
                    log(Error) << "Category '" << categoryName << "' does not exist!" << endlog();
                }
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
        log(Info)
            << "Category '" << (*iter)->getName() << "', level="
            << log4cpp::Priority::getPriorityName((*iter)->getPriority())
            << ", typeid='"
            << typeid(*iter).name()
            << "', type really is '" 
            << std::string(0 != dynamic_cast<OCL::logging::Category*>(*iter)
                           ? "OCL::Category" : "log4cpp::Category")
            << "'" << endlog();
    }
}
   
// namespaces
}
}

ORO_CREATE_COMPONENT_TYPE();
ORO_LIST_COMPONENT_TYPE(OCL::logging::LoggingService);
