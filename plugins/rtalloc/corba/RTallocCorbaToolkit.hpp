/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/
#ifndef __RTALLOCCORBATOOLKIT_HPP
#define __RTALLOCCORBATOOLKIT_HPP 1

#include <rtt/types/TransportPlugin.hpp>

namespace RTT
{
    namespace Corba
    {

    class CorbaRTallocPlugin : public RTT::types::TransportPlugin
    {
    public:
        bool registerTransport(std::string name, RTT::types::TypeInfo* ti);
        
        std::string getTransportName() const;
        
        std::string getName() const;
    };

    // the global instance
    extern CorbaRTallocPlugin     corbaRTallocPlugin;

// namespace
}
}


#endif
