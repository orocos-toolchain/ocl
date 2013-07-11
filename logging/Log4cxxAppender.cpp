#include "logging/Log4cxxAppender.hpp"
#include "log4cxx/spi/loggingevent.h"
#include "rtt/Component.hpp"
#include <rtt/Logger.hpp>

using namespace RTT;

namespace OCL {
namespace logging {
    using namespace log4cxx;

    log4cxx::LevelPtr tolog4cxxLevel( log4cpp::Priority::Value priority) {
        switch (priority) {
        //case log4cpp::Priority::EMERG : EMERG and FATAL are both zero
            //return Level::getFatal();
        case log4cpp::Priority::FATAL :
            return Level::getFatal();
        case log4cpp::Priority::ALERT :
            return Level::getError();
        case log4cpp::Priority::CRIT :
            return Level::getError();
        case log4cpp::Priority::ERROR :
            return Level::getError();
        case log4cpp::Priority::WARN :
            return Level::getWarn();
        case log4cpp::Priority::NOTICE :
            return Level::getInfo();
        case log4cpp::Priority::INFO :
            return Level::getInfo();
        case log4cpp::Priority::DEBUG :
            return Level::getDebug();
        case log4cpp::Priority::NOTSET :
            return Level::getDebug();
        }
        return Level::getDebug();
    }

    spi::LoggingEventPtr tolog4cxx(logging::LoggingEvent const& e, log4cxx::helpers::Pool & pool)
    {
        return spi::LoggingEventPtr(new spi::LoggingEvent(makeString( e.categoryName ), tolog4cxxLevel(e.priority), makeString( e.message ), log4cxx::spi::LocationInfo("filename", "functionname", 0)));
    }

Log4cxxAppender::Log4cxxAppender(std::string name) :
    RTT::TaskContext(name,RTT::TaskContext::PreOperational),
        socketAppender(0),
        hostname_prop("localhost"), port_prop(4560),
        maxEventsPerCycle_prop(0),
        maxEventsPerCycle(0)
{

    ports()->addEventPort("LogPort", log_port );
    properties()->addProperty("Hostname",hostname_prop).doc("Host name where ChainSaw or other logging server runs.");
    properties()->addProperty("MaxEventsPerCycle",maxEventsPerCycle_prop).doc("Maximum number of log events to pop per cycle");
    properties()->addProperty("Port", port_prop).doc("Logging server port to use on Hostname. ChainSaw uses port 4560.");
}

Log4cxxAppender::~Log4cxxAppender()
{
}

bool Log4cxxAppender::configureHook()
{
    // verify valid limits
    int m = maxEventsPerCycle_prop;
    if ((0 > m))
    {
        log(Error) << "Invalid maxEventsPerCycle value of "
                   << m << ". Value must be >= 0."
                   << endlog();
        return false;
    }
    maxEventsPerCycle = m;

    // \todo error checking
    if ( hostname_prop == "localhost")
        address = helpers::InetAddress::getLocalHost();
    else
        address = helpers::InetAddress::getByName( hostname_prop );

    if ( socketAppender ) {
        socketAppender->close();
        delete socketAppender;
    }

    net::SocketAppender::DEFAULT_RECONNECTION_DELAY = 3000; // 3s

    socketAppender = new
        log4cxx::net::SocketAppender( address, port_prop );

    socketAppender->activateOptions(p);

    return true;
}

void Log4cxxAppender::updateHook()
{
    if (!log_port.connected()) return;      // no category connected to us

    /* Consume waiting events until
       a) the buffer is empty
       b) we consume too many events on one cycle
     */
    OCL::logging::LoggingEvent   event;
    assert(socketAppender);
    assert(0 <= maxEventsPerCycle);
    if (0 == maxEventsPerCycle)
    {
        // consume infinite events
        for (;;)
        {
            if (log_port.read( event ) == NewData)
            {
                spi::LoggingEventPtr e2 = tolog4cxx( event, p );
                socketAppender->doAppend(e2, p);
            }
            else
            {
                break;      // nothing to do
            }
        }
    }
    else
    {

        // consume up to maxEventsPerCycle events
        int n       = maxEventsPerCycle;
        do
        {
            if (log_port.read( event ) == NewData)
            {
                spi::LoggingEventPtr e2 = tolog4cxx( event, p );
                socketAppender->doAppend(e2, p);
            }
            else
            {
                break;      // nothing to do
            }
            --n;
        }
        while (0 < n);
    }
}

void Log4cxxAppender::cleanupHook()
{
    /* normally in log4cpp the category owns the appenders and deletes them
       itself, however we don't associate appenders and categories in the
       same manner. Hence, you have to manually manage appenders.
    */
    socketAppender->close();
    delete socketAppender;
    socketAppender = 0;
}

// namespaces
}
}

ORO_CREATE_COMPONENT(OCL::logging::Log4cxxAppender);
