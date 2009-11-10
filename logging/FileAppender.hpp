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
    virtual bool configureHook();
	virtual void updateHook();
	virtual void cleanupHook();
    
    /// Name of file to append to
    RTT::Property<std::string>      filename_prop;
};

// namespaces
}
}

#endif
