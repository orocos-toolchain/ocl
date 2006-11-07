
#include "ConsoleReporting.hpp"
#include "rtt/Logger.hpp"
#include "rtt/marsh/TableMarshaller.hpp"
#include "rtt/marsh/TableHeaderMarshaller.hpp"

namespace OCL
{
    using namespace RTT;
    using namespace std;

    ConsoleReporting::ConsoleReporting(std::string fr_name /*= "Reporting"*/, std::ostream& console /*= std::cerr*/)
        : ReportingComponent( fr_name ),
          mconsole( console )
    {
    }

        bool ConsoleReporting::startup()
        {
            RTT::Logger::In in("ConsoleReporting::startup");
            if (mconsole) {
                RTT::Marshaller* fheader;
                RTT::Marshaller* fbody;
                if ( this->writeHeader)
                    fheader = new RTT::TableHeaderMarshaller<std::ostream>( mconsole );
                else 
                    fheader = 0;
                fbody = new RTT::TableMarshaller<std::ostream>( mconsole );
                
                this->addMarshaller( fheader, fbody );
            } else {
                RTT::Logger::log() <<RTT::Logger::Error << "Could not write to console for reporting."<<RTT::Logger::endl;
            }

            return ReportingComponent::startup();
        }

        void ConsoleReporting::shutdown()
        {
            ReportingComponent::shutdown();

            this->removeMarshallers();
        }

        bool ConsoleReporting::screenComponent( const std::string& comp)
        {
            if ( !mconsole )
                return false;
            return this->screenImpl( comp, mconsole );
        }
}

