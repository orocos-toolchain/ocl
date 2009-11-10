#ifndef	TESTLOGGINGAVAILABILITY_HPP
#define	TESTLOGGINGAVAILABILITY_HPP 1

#include <rtt/TaskContext.hpp>

namespace OCL {
namespace logging {

// forward declare
class Category;

namespace test {

class LoggingAvailability : public RTT::TaskContext
{
public:
	LoggingAvailability(std::string name);
	virtual ~LoggingAvailability();

protected:
    virtual bool configureHook();
    virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();

    OCL::logging::Category* logger;
};

// namespaces
}
}
}

#endif
