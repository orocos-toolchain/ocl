
#include "FileReporting.hpp"
#include "rtt/RTT.hpp"
#include "rtt/Logger.hpp"
#include "rtt/marsh/TableMarshaller.hpp"
#include "rtt/marsh/TableHeaderMarshaller.hpp"

namespace OCL
{
    using namespace RTT;
    using namespace std;

    FileReporting::FileReporting(const std::string& fr_name)
        : ReportingComponent( fr_name ),
          repfile("ReportFile","Location on disc to store the reports.", "results.txt")
    {
        this->properties()->addProperty( &repfile );
    }

    bool FileReporting::startup()
    {
        mfile.open( repfile.get().c_str() );
        if (mfile) {
            if ( this->writeHeader)
                fheader = new RTT::TableHeaderMarshaller<std::ostream>( mfile );
            else 
                fheader = 0;
            fbody = new RTT::TableMarshaller<std::ostream>( mfile );
                
            this->addMarshaller( fheader, fbody );
        } else {
            Logger::log() <<Logger::Error << "Could not open file "+repfile.get()+" for reporting."<<Logger::endl;
        }

        return ReportingComponent::startup();
    }

    void FileReporting::shutdown()
    {
        ReportingComponent::shutdown();

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
