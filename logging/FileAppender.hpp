#ifndef	FILEAPPENDER_HPP
#define	FILEAPPENDER_HPP 1

#include "Appender.hpp"
#include <rtt/Property.hpp>

namespace OCL {
namespace logging {

class FileAppender : public OCL::logging::Appender
{
public:
	FileAppender(std::string name);
	virtual ~FileAppender();
protected:
	/// Create log4cpp appender
    virtual bool configureHook();
	/// Process at most \a maxEventsPerCycle event
	virtual void updateHook();
	/// Destroy appender
	virtual void cleanupHook();
    
    /// Name of file to append to
    RTT::Property<std::string>      filename_prop;
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
