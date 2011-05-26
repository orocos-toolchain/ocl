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
        ndc(""),
        priority(log4cpp::Priority::NOTSET),
        threadName(""),
        timeStamp()
{
}

LoggingEvent::LoggingEvent(const LoggingEvent& toCopy) :
        categoryName(toCopy.categoryName),
        message(toCopy.message),
        ndc(toCopy.ndc),
        priority(toCopy.priority),
        threadName(toCopy.threadName),
        timeStamp(toCopy.timeStamp)
{
}

LoggingEvent::LoggingEvent(const rt_string& categoryName, 
                           const rt_string& message,
                           const rt_string& ndc, 
                           log4cpp::Priority::Value priority) :
        categoryName(categoryName),
        message(message),
        ndc(ndc),
        priority(priority),
        threadName(""),
        timeStamp()
{
    char    buffer[16];
    threadName = log4cpp::threading::getThreadId(&buffer[0]);
}

const LoggingEvent& LoggingEvent::operator=(const LoggingEvent& rhs)
{
    if (&rhs != this)   // prevent self-copy
    {
        categoryName    = rhs.categoryName;
        message         = rhs.message;
        ndc             = rhs.ndc;
        priority        = rhs.priority;
        threadName      = rhs.threadName;
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
                                 makeString(this->ndc),
                                 this->priority,
                                 makeString(this->threadName),
                                 this->timeStamp);
}
        

// namespaces
}
}
