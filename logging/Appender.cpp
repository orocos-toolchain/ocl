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
        layoutPattern_prop("LayoutPattern", "Layout conversion pattern (for those layouts that use a pattern)"),
        countMaxPopped(0)
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

void Appender::stopHook()
{
	drainBuffer();

	// introduce event to log diagnostics
	if (0 != appender)
	{
		/* place a "#" at the front of the message, for appenders that are
		 reporting data for post-processing. These particular appenders
		 don't prepend the time data (it's one at time of sampling).
		 This way gnuplot, etc., ignore this diagnostic data.
		*/
		std::stringstream	ss;
		ss << "# countMaxPopped=" << countMaxPopped;
		log4cpp::LoggingEvent	event("OCL.logging.Appender",
									  ss.str(),
									  "",
									  log4cpp::Priority::DEBUG);
		appender->doAppend(event);
	}
}

void Appender::drainBuffer()
{
	processEvents(0);
}

void Appender::processEvents(int n)
{
    if (!log_port.connected()) return;      // no category connected to us
	if (!appender) return;				// no appender!?

	// check pre-conditions
	if (0 > n) n = 1;

    /* Consume waiting events until
       a) the buffer is empty
       b) we consume enough events
	*/
    OCL::logging::LoggingEvent	event;
	bool 						again = false;
	int							count = 0;

	do
	{
        if (log_port.read( event ) == RTT::NewData)
		{
			++count;

			appender->doAppend( event.toLog4cpp() );

			// Consume infinite events OR up to n events
			again = (0 == n) || (count < n);
			if ((0 != n) && (count == n)) ++countMaxPopped;
		}
		else
		{
			break;      // nothing to do
		}
	}
	while (again);
}

// namespaces
}
}

ORO_LIST_COMPONENT_TYPE(OCL::logging::Appender);
