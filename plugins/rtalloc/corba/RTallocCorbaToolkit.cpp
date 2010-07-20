/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/
#include "RTallocCorbaToolkit.hpp"
#include <rtt/types/TypekitPlugin.hpp>
#include <rtt/transports/corba/CorbaTemplateProtocol.hpp>
#include "RTallocCorbaConversion.hpp"

using namespace RTT;
using namespace RTT::detail;

namespace RTT
{
namespace corba
{
    
    bool CorbaRTallocPlugin::registerTransport(std::string name, types::TypeInfo* ti)
    {
        assert( name == ti->getTypeName() );
        // name must match that in plugin::loadTypes() and 
        // typeInfo::composeTypeInfo(), etc
        if ( name == "rtstring" )
            return ti->addProtocol(ORO_CORBA_PROTOCOL_ID, new corba::CorbaTemplateProtocol< OCL::String >() );
        return false;
    }

    std::string CorbaRTallocPlugin::getTransportName() const {
        return "CORBA";
    }

    std::string CorbaRTallocPlugin::getTypekitName() const {
		return "rtalloc-types";
    }

    std::string CorbaRTallocPlugin::getName() const {
		return "CorbaRTalloc";
    }

// namespace
}
}

ORO_TYPEKIT_PLUGIN(RTT::corba::CorbaRTallocPlugin);

