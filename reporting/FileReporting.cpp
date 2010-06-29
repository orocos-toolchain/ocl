
#include "FileReporting.hpp"
#include <rtt/RTT.hpp>
#include <rtt/Logger.hpp>
#include "TableMarshaller.hpp"
#include "NiceHeaderMarshaller.hpp"


#include "ocl/Component.hpp"
ORO_LIST_COMPONENT_TYPE(OCL::FileReporting)

namespace OCL
{
    using namespace RTT;
    using namespace std;

    FileReporting::FileReporting(const std::string& fr_name)
        : ReportingComponent( fr_name ),
          repfile("ReportFile","Location on disc to store the reports.", "reports.dat")
    {
        this->properties()->addProperty( repfile );
    }

    bool FileReporting::startHook()
    {
        mfile.open( repfile.get().c_str() );
        if (mfile) {
            if ( this->writeHeader)
                fheader = new RTT::NiceHeaderMarshaller<std::ostream>( mfile );
            else
                fheader = 0;
            fbody = new RTT::TableMarshaller<std::ostream>( mfile );

            this->addMarshaller( fheader, fbody );
        } else {
            log(Error) << "Could not open file "+repfile.get()+" for reporting."<<endlog();
        }

        return ReportingComponent::startHook();
    }

    void FileReporting::stopHook()
    {
        ReportingComponent::stopHook();

        this->removeMarshallers();
        if (mfile)
            mfile.close();
    }

    bool FileReporting::screenComponent( const std::string& comp)
    {
        Logger::In in("FileReporting::screenComponent");
        ofstream file( (comp + ".screen").c_str() );
        if (!file)
            return false;
        return this->screenImpl( comp, file );
    }

}
