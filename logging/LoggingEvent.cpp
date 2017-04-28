#include "LoggingEvent.hpp"
#include <log4cpp/Priority.hh>
#include <log4cpp/threading/Threading.hh>
#include <cstdio>

using namespace RTT;

namespace OCL {
namespace logging {
    
LoggingEvent::LoggingEvent() :
        categoryName(""),
        message(""),
        priority(log4cpp::Priority::NOTSET),
        timeStamp()
{
    threadName[0] = '\0';       // ensure is terminated
}

LoggingEvent::LoggingEvent(const LoggingEvent& toCopy) :
        categoryName(toCopy.categoryName),
        message(toCopy.message),
        priority(toCopy.priority),
        timeStamp(toCopy.timeStamp)
{
    memcpy(threadName, toCopy.threadName, THREADNAME_SIZE);
}

LoggingEvent::LoggingEvent(const rt_string& categoryName, 
                           const rt_string& message,
                           log4cpp::Priority::Value priority) :
        categoryName(categoryName),
        message(message),
        priority(priority),
        timeStamp()
{
    /// See also log4cpp/src/PThreads.cpp::getThreadId()
    (void)log4cpp::threading::getThreadId(&threadName[0]);
}

LoggingEvent::LoggingEvent(const std::string& c,
                           const rt_string& m,
                           log4cpp::Priority::Value priority) :
        /* Optimization with std::string to prevent need to walk null-terminated
         * string.
         */
        categoryName(c.c_str(), c.size()),
        message(m),
        priority(priority),
        timeStamp()
{
    /// See also log4cpp/src/PThreads.cpp::getThreadId()
    (void)log4cpp::threading::getThreadId(&threadName[0]);
}

const LoggingEvent& LoggingEvent::operator=(const LoggingEvent& rhs)
{
    if (&rhs != this)   // prevent self-copy
    {
        categoryName    = rhs.categoryName;
        message         = rhs.message;
        priority        = rhs.priority;
        memcpy(threadName, rhs.threadName, THREADNAME_SIZE);
        timeStamp		= rhs.timeStamp;
    }
    return *this;
}

LoggingEvent::~LoggingEvent()
{
}

log4cpp::LoggingEvent LoggingEvent::toLog4cpp()
{   
    return log4cpp::LoggingEvent(makeString(this->categoryName),
                                 makeString(this->message),
                                 makeString(""),    // not used
                                 this->priority,
                                 this->threadName,
                                 this->timeStamp);
}
        

// namespaces
}
}
