#ifndef	GENERATIONALFILEAPPENDER_HPP
#define	GENERATIONALFILEAPPENDER_HPP 1

#include "Appender.hpp"
#include <rtt/Property.hpp>

namespace OCL {
namespace logging {

/** Appender supporting generations of log files

	Each new generation is logged to a new file
*/
class GenerationalFileAppender : public OCL::logging::Appender
{
public:
	GenerationalFileAppender(std::string name);
	virtual ~GenerationalFileAppender();
protected:
	/// Create log4cpp appender
    virtual bool configureHook();
	/// Process at most \a maxEventsPerCycle event
	virtual void updateHook();
	/// Destroy appender
	virtual void cleanupHook();

	/// @copydoc OCL::logging::advanceGeneration()
	RTT::Operation<void(void)>		advanceGeneration_op;
	/// Advance to the next logfile generation
	void advanceGeneration();

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
    int								maxEventsPerCycle;
};

// namespaces
}
}

#endif
