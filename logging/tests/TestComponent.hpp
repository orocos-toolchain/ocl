#ifndef	TESTCOMPONENT_HPP
#define	TESTCOMPONENT_HPP 1

#include <rtt/TaskContext.hpp>

namespace OCL {
namespace logging {

// forward declare
class Category;

namespace test {

class Component : public RTT::TaskContext
{
public:
	Component(std::string name);
	virtual ~Component();

protected:
    virtual bool startHook();
	virtual void updateHook();

    /// Name of our category
    std::string             categoryName;
    /// Our logging category
    OCL::logging::Category* logger;
};

// namespaces
}
}
}

#endif
