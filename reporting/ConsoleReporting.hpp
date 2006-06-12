#ifndef ORO_COMP_CONSOLE_REPORTING_HPP
#define ORO_COMP_CONSOLE_REPORTING_HPP

#include "ReportingComponent.hpp"
#include <iostream>

namespace Orocos
{
    /**
     * A component which writes data reports to a console.
     */
    class ConsoleReporting
        : public ReportingComponent
    {
    protected:
        /**
         * Console to write reports to.
         */
        std::ostream& mconsole;

    public:
        /**
         * Create a reporting component which writes to a C++ stream.
         */
        ConsoleReporting(std::string fr_name = "ReportingComponent", std::ostream& console = std::cerr);

        bool startup();

        void shutdown();

        /**
         * Writes the interface status of \a comp to 
         * the console.
         */
        bool screenComponent( const std::string& comp);

    };

}

#endif
