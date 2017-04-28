#ifndef _LOGGINGEVENT_HPP 
#define _LOGGINGEVENT_HPP 1

#include <rtt/rt_string.hpp>
#include <log4cpp/LoggingEvent.hh>

namespace OCL {
namespace logging {

/// A mirror of log4cpp::LoggingEvent, except using real-time capable strings
struct LoggingEvent 
{
public:
    /// Construct from RT strings
    LoggingEvent(const RTT::rt_string& category, 
                 const RTT::rt_string& message, 
                 log4cpp::Priority::Value priority);
    /** Construct from a mix of types
        This is an optimization for use by \a Category::_logUnconditionally2(),
        which constructs from "std::string&, rt_string&".
     */
    LoggingEvent(const std::string& category,
                 const RTT::rt_string& message,
                 log4cpp::Priority::Value priority);
    /// Create with empty values
    LoggingEvent();
    // copy constructor
    LoggingEvent(const LoggingEvent& toCopy);
    // assignment operator
    const LoggingEvent& operator=(const LoggingEvent& rhs);
    ~LoggingEvent();

    /*const */RTT::rt_string       categoryName;

    /*const */RTT::rt_string       message;

    /* NB "ndc" is not used in the real-time logging framework as it is
     * inherently non-real-time safe in log4cpp itself. Instead an empty string
     * is passed through. To prevent the repetitive use of an empty string,
     * just don't use this at all and pass the empty string explicitly back
     * to log4cpp when converting to log4cpp in toLog4cpp().
     */
    /*const RTT::rt_string       ndc;*/

    log4cpp::Priority::Value    priority;

    /// The maximum size of threadname (bascially - pthread_self() as "%ld")
    /// See also log4cpp/src/PThreads.cpp::getThreadId()
    static const size_t         THREADNAME_SIZE=16;

    char                        threadName[THREADNAME_SIZE];

    log4cpp::TimeStamp          timeStamp;

    /// Convert to log4cpp class
    /// \warning not realtime
    log4cpp::LoggingEvent toLog4cpp();
};

// namespaces
}
}

#endif

