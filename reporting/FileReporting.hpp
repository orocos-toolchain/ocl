#ifndef ORO_COMP_FILE_REPORTING_HPP
#define ORO_COMP_FILE_REPORTING_HPP

#include "ReportingComponent.hpp"
#include <fstream>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * A component which writes data reports to a file.
     */
    class FileReporting
        : public ReportingComponent
    {
    protected:
        /**
         * File name to write reports to.
         */
        RTT::Property<std::string>   repfile;

        /**
         * File to write reports to.
         */
        std::ofstream mfile;

        RTT::marsh::MarshallInterface* fheader;
        RTT::marsh::MarshallInterface* fbody;
    public:
        FileReporting(const std::string& fr_name);

        bool startHook();

        void stopHook();

        /**
         * Writes the interface status of \a comp to
         * a file 'comp.screen'.
         */
        bool screenComponent( const std::string& comp);
    };
}

#endif
