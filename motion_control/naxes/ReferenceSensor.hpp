#ifndef _REFERENCESENSOR_HPP_
#define _REFERENCESENSOR_HPP_

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Ports.hpp>

namespace Orocos
{
    class ReferenceSensor : public RTT::GenericTaskContext
    {
        int nrofaxes;
    public:
        ReferenceSensor(std::string name="ReferenceSensor",int _nrofaxes=6);
                
        virtual bool getReference(int axis);
        
        virtual ~ReferenceSensor() {};
        
        std::vector<RTT::ReadDataPort<bool>*>  reference;	
    };
}

#endif
