#include "logging/Appender.hpp"
#include "ocl/Component.hpp"

#include <log4cpp/Appender.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/SimpleLayout.hh>
#include <log4cpp/PatternLayout.hh>

namespace OCL {
namespace logging {

Appender::Appender(std::string name) :
    RTT::TaskContext(name, RTT::TaskContext::PreOperational), 
        appender(0),
        layoutName_prop("LayoutName", "Layout name (e.g. 'simple', 'pattern')"),
        layoutPattern_prop("LayoutPattern", "Layout conversion pattern (for those layouts that use a pattern)")
{
    ports()->addEventPort("LogPort", log_port );

    properties()->addProperty(layoutName_prop);
    properties()->addProperty(layoutPattern_prop);
}

Appender::~Appender()
{
}

bool Appender::configureLayout()
{
    bool rc;
    const std::string& layoutName       = layoutName_prop.rvalue();
    const std::string& layoutPattern    = layoutPattern_prop.rvalue();

    rc = true;          // prove otherwise
    if (appender && 
        (!layoutName.empty()))
    {
        // \todo layout factory??
        if (0 == layoutName.compare("basic"))
        {
            appender->setLayout(new log4cpp::BasicLayout());
        }
        else if (0 == layoutName.compare("simple"))
        {
            appender->setLayout(new log4cpp::SimpleLayout());
        }
        else if (0 == layoutName.compare("pattern")) 
        {
            log4cpp::PatternLayout *layout = new log4cpp::PatternLayout();
            /// \todo ensure "" != layoutPattern?
            layout->setConversionPattern(layoutPattern);
			appender->setLayout(layout);
            // the layout is now owned by the appender, and will be deleted
            // by it when the appender is destroyed
        }
        else 
        {
            RTT::log(RTT::Error) << "Invalid layout '" << layoutName
                       << "' in configuration for category: "
                       << getName() << RTT::endlog();
            rc = false;
        }
    }

    return rc;
}
    
bool Appender::startHook()
{
    /// \todo input ports must be connected?
//    return log_port.ready();  

    return true;
}

// namespaces
}
}

ORO_LIST_COMPONENT_TYPE(OCL::logging::Appender);
