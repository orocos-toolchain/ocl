#ifndef ORO_COMP_NETCDF_REPORTING_HPP
#define ORO_COMP_NETCDF_REPORTING_HPP

#include "ReportingComponent.hpp"

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * A component which writes data reports to a netCDF file.
     */
    class NetcdfReporting
        : public ReportingComponent
    {
    protected:

        /**
         * File name of netCDF file.
         */
        RTT::Property<std::string>  repfile;

        /**
         * Netcdf ID
         */
        int ncid;
        /**
         * Dimension ID of unlimited dimension
         */
        int dimsid;

        RTT::marsh::MarshallInterface* fheader;
        RTT::marsh::MarshallInterface* fbody;
    public:
        NetcdfReporting(const std::string& fr_name);

        bool startHook();

        void stopHook();

    };
}

#endif

