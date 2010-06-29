
#include "ConsoleReporting.hpp"
#include <rtt/Logger.hpp>
#include "TableMarshaller.hpp"
#include "NiceHeaderMarshaller.hpp"

#include "ocl/Component.hpp"
ORO_LIST_COMPONENT_TYPE(OCL::ConsoleReporting)

namespace OCL
{
    using namespace RTT;
    using namespace std;

    ConsoleReporting::ConsoleReporting(std::string fr_name /*= "Reporting"*/, std::ostream& console /*= std::cerr*/)
        : ReportingComponent( fr_name ),
          mconsole( console )
    {
    }

        bool ConsoleReporting::startHook()
        {
            RTT::Logger::In in("ConsoleReporting::startup");
            if (mconsole) {
                RTT::marsh::MarshallInterface* fheader;
                RTT::marsh::MarshallInterface* fbody;
                if ( this->writeHeader)
                    fheader = new RTT::NiceHeaderMarshaller<std::ostream>( mconsole );
                else
                    fheader = 0;
                fbody = new RTT::TableMarshaller<std::ostream>( mconsole );

                this->addMarshaller( fheader, fbody );
            } else {
                log(Error) << "Could not write to console for reporting."<<RTT::endlog();
            }

            return ReportingComponent::startHook();
        }

        void ConsoleReporting::stopHook()
        {
            ReportingComponent::stopHook();

            this->removeMarshallers();
        }

        bool ConsoleReporting::screenComponent( const std::string& comp)
        {
            if ( !mconsole )
                return false;
            return this->screenImpl( comp, mconsole );
        }
}

