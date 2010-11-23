#include <rtt/Service.hpp>
#include <rtt/Logger.hpp>
#include <iostream>

#include <rtt/types/GlobalsRepository.hpp>
#include <rtt/types/Types.hpp>
#include <rtt/types/TypeInfoName.hpp>
#include <rtt/plugin/ServicePlugin.hpp>
#include "OCL.hpp"

namespace OCL
{

    /**
     * A service that provides basic printing to std::cout, std::cerr and the RTT::Logger.
     * Can be loaded in scripts by writing 'requires("print")' on top of the file.
     */
    class PrintService: public RTT::Service
    {
    public:
        PrintService(TaskContext* parent) :
            RTT::Service("print", parent)
        {
            doc("A service that provides basic printing to std::cout, std::cerr and the RTT::Logger.");
            // add the operations
            addOperation("ln", &PrintService::println, this).doc(
                    "Prints a line to standard output.").arg("line",
                    "A string. Use a '+' to mix strings with numbers/variables.");
            addOperation("err", &PrintService::printerr, this).doc(
                    "Prints a line to standard error.").arg("line",
                    "A string. Use a '+' to mix strings with numbers/variables.");
            addOperation("log", &PrintService::printlog, this).doc(
                    "Prints a line to Orocos logger class.").arg("level","The LogLevel to use.").arg("line",
                    "A string. Use a '+' to mix strings with numbers/variables.");

            // add the log-levels as global variables.
            if (types::Types()->type("LogLevel") == 0) {
                types::Types()->addType( new types::TypeInfoName<Logger::LogLevel>("LogLevel") );
                types::GlobalsRepository::shared_ptr globals = types::GlobalsRepository::Instance();


                // Data Flow enums:
                globals->setValue( new Constant<Logger::LogLevel>("Never",Logger::Never) );
                globals->setValue( new Constant<Logger::LogLevel>("Error",Logger::Error) );
                globals->setValue( new Constant<Logger::LogLevel>("Fatal",Logger::Fatal) );
                globals->setValue( new Constant<Logger::LogLevel>("Critical",Logger::Critical) );
                globals->setValue( new Constant<Logger::LogLevel>("Warning",Logger::Warning) );
                globals->setValue( new Constant<Logger::LogLevel>("Info",Logger::Info) );
                globals->setValue( new Constant<Logger::LogLevel>("Debug",Logger::Debug) );
                globals->setValue( new Constant<Logger::LogLevel>("RealTime",Logger::RealTime) );
            }
        }

        void println(const std::string& arg)
        {
            std::cout << arg << std::endl;
        }
        void printerr(const std::string& arg)
        {
            std::cerr << arg << std::endl;
        }
        void printlog(Logger::LogLevel level, const std::string& arg)
        {
            log(LoggerLevel(level)) << arg <<endlog();
        }
    };
}

ORO_SERVICE_NAMED_PLUGIN( OCL::PrintService, "print")

