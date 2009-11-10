#ifndef _LOGGINGEVENT_HPP 
#define _LOGGINGEVENT_HPP 1

#include "String.hpp"   // OCL/rtalloc
#include <log4cpp/LoggingEvent.hh>

namespace OCL {
namespace logging {

/// A mirror of log4cpp::LoggingEvent, except using real-time capable strings
struct LoggingEvent 
{
public:
    LoggingEvent(const OCL::String& category, 
                 const OCL::String& message, 
                 const OCL::String& ndc, 
                 log4cpp::Priority::Value priority);
    /// Create with empty values
    LoggingEvent();
    // copy constructor
    LoggingEvent(const LoggingEvent& toCopy);
    // assignment operator
    const LoggingEvent& operator=(const LoggingEvent& rhs);
    ~LoggingEvent();

    /*const */OCL::String       categoryName;

    /*const */OCL::String       message;

    /*const */OCL::String       ndc;

    log4cpp::Priority::Value    priority;

    /*const */OCL::String       threadName;

    log4cpp::TimeStamp          timeStamp;

    /// Convert to log4cpp class
    /// \warning not realtime
    log4cpp::LoggingEvent toLog4cpp();
};

// namespaces
}
}

#endif

