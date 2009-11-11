/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/
#include "RTallocCorbaToolkit.hpp"
#include <rtt/Types.hpp>
#include <rtt/Toolkit.hpp>
#include <rtt/Plugin.hpp>
#include <rtt/corba/CorbaTemplateProtocol.hpp>
#include "RTallocCorbaConversion.hpp"

using namespace RTT;
using namespace RTT::detail;
using namespace RTT::Corba;

namespace RTT
{
    namespace Corba
    {
    
    bool CorbaRTallocPlugin::registerTransport(std::string name, TypeInfo* ti)
    {
        assert( name == ti->getTypeName() );
        // name must match that in plugin::loadTypes() and 
        // typeInfo::composeTypeInfo(), etc
        if ( name == "rtstring" )
            return ti->addProtocol(ORO_CORBA_PROTOCOL_ID, new CorbaTemplateProtocol< OCL::String >() );
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

ORO_TOOLKIT_PLUGIN(RTT::Corba::corbaRTallocPlugin);

