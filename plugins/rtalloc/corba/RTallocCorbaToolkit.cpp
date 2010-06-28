/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/
#include "RTallocCorbaToolkit.hpp"
#include <rtt/types/Types.hpp>
#include <rtt/types/TypekitRepository.hpp>
#include <rtt/plugin/Plugin.hpp>
#include <rtt/transports/corba/CorbaTemplateProtocol.hpp>
#include "RTallocCorbaConversion.hpp"

using namespace RTT;
using namespace RTT::detail;
using namespace RTT::corba;

namespace RTT
{
    namespace Corba
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

    std::string CorbaRTallocPlugin::getName() const {
		return "CorbaRTalloc";
    }

    CorbaRTallocPlugin corbaRTallocPlugin;

// namespace
}
}

ORO_TOOLKIT_PLUGIN(RTT::corba::corbaRTallocPlugin);

