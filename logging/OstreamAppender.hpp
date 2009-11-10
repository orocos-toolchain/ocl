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
};

// namespaces
}
}

#endif
