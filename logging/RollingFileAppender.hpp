#ifndef	ROLLINGFILEAPPENDER_HPP
#define	ROLLINGFILEAPPENDER_HPP 1

#include "Appender.hpp"
#include <rtt/Property.hpp>

namespace OCL {
namespace logging {

class RollingFileAppender : public OCL::logging::Appender
{
public:
	RollingFileAppender(std::string name);
	virtual ~RollingFileAppender();
protected:
	/// Create log4cpp appender
    virtual bool configureHook();
	/// Process at most \a maxEventsPerCycle event
	virtual void updateHook();
	/// Destroy appender
	virtual void cleanupHook();
    
    /// Name of file to append to
    RTT::Property<std::string>      filename_prop;
    /// Maximum file size (in bytes) before rolling over
    RTT::Property<int>      		maxFileSize_prop;
    /// Maximum number of backup files to keep
    RTT::Property<int>      		maxBackupIndex_prop;
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
