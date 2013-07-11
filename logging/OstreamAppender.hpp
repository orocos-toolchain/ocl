#ifndef	OSTREAMAPPENDER_HPP
#define	OSTREAMAPPENDER_HPP 1

#include "Appender.hpp"
#include <rtt/Property.hpp>

namespace OCL {
namespace logging {

class OstreamAppender : public OCL::logging::Appender
{
public:
	OstreamAppender(std::string name);
	virtual ~OstreamAppender();
protected:
    virtual bool configureHook();
	virtual void updateHook();
	virtual void cleanupHook();

    /** 
     * Property to set maximum number of log events to pop per cycle
     */
    RTT::Property<int>              maxEventsPerCycle_prop;

    /** 
     * Maximum number of log events to pop per cycle
     *
     * Defaults to 1.
     *
     * A value of 0 indicates to not limit the number of events per cycle.
     * With enough event production, this could lead to thread
     * starvation!
     */
    int                           maxEventsPerCycle;
};

// namespaces
}
}

#endif
