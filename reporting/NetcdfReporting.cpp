#include "NetcdfReporting.hpp"
#include <rtt/RTT.hpp>
#include <rtt/Logger.hpp>
#include <rtt/types/Types.hpp>
#include <rtt/types/TemplateTypeInfo.hpp>
#include "NetcdfMarshaller.hpp"
#include "NetcdfHeaderMarshaller.hpp"

#include "ocl/Component.hpp"
ORO_CREATE_COMPONENT(OCL::NetcdfReporting)

#include <netcdf.h>

namespace OCL
{
    using namespace RTT;
    using namespace std;

    NetcdfReporting::NetcdfReporting(const std::string& fr_name)
        : ReportingComponent( fr_name ),
          repfile("ReportFile","Location on disc to store the reports.", "reports.nc")
    {
        this->properties()->addProperty( repfile );

        if(types::TypeInfoRepository::Instance()->getTypeInfo<short>() == 0 )
        {
        	types::TypeInfoRepository::Instance()->addType(new types::TemplateTypeInfo<short, true>("short"));
        }
    }

    bool NetcdfReporting::startHook()
    {
      int retval;

      /**
       * Create a new netcdf dataset in the NC_CLOBBER mode.
       * This means that the nc_create function overwrites any existing dataset.
       */
      retval = nc_create(repfile.get().c_str(), NC_CLOBBER | NC_SHARE, &ncid);
      if ( retval ) {
       log(Error) << "Could not create "+repfile.get()+" for reporting."<<endlog();
       return false;
      }

      /**
       * Create a new dimension to an open netcdf dataset, called time.
       * Size NC_UNLIMITED indicates that the length of this dimension is undefined.
       */
      retval = nc_def_dim(ncid, "time", NC_UNLIMITED, &dimsid);
      if ( retval ) {
       log(Error) << "Could not create time dimension "+repfile.get() <<endlog();
       return false;
      }

      /**
       * Leave define mode and enter data mode.
       */
      retval = nc_enddef( ncid );
      if ( retval ) {
       log(Error) << "Could not leave define mode in "+repfile.get() <<endlog();
       return false;
      }

      fheader = new RTT::NetcdfHeaderMarshaller( ncid , dimsid);
      fbody = new RTT::NetcdfMarshaller( ncid );
                
      this->addMarshaller( fheader, fbody );

      return ReportingComponent::startHook();
    }

  void NetcdfReporting::stopHook()
  {
    int retval;

    ReportingComponent::stopHook();

    this->removeMarshallers();

    /**
     * Close netcdf dataset
     */
    if ( ncid )
      retval = nc_close (ncid);
    if ( retval )
      log(Error) << "Could not close file "+repfile.get()+" for reporting."<<endlog();
  }

}
