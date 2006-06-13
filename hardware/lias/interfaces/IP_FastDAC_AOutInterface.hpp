#ifndef IP_FASTDAC_AOUTINTERFACE_HPP
#define IP_FASTDAC_AOUTINTERFACE_HPP


#include "../drivers/LiAS_lxrt_user.h"
#include <device_interface/AnalogOutInterface.hpp>
#include <iostream>

class IP_FastDAC_AOutInterface : public ORO_DeviceInterface::AnalogOutInterface<unsigned int>
{
public:
    IP_FastDAC_AOutInterface(const std::string& name);
    IP_FastDAC_AOutInterface();
    virtual ~IP_FastDAC_AOutInterface() {};

    virtual void rangeSet(unsigned int chan, unsigned int range)
    {
        IP_FastDAC_set_gain_of_group( chan/2 + 1, range);
    }

    virtual void arefSet(unsigned int chan, unsigned int aref) {};

    virtual void write( unsigned int chan, unsigned int value )
    { 
        //std::cout << "IP_FastDAC_AOutInterface: write " << value << std::endl;
        IP_FastDAC_write_to_channel (value, chan);
    }

    virtual unsigned int binaryRange() const { return 8192; }

    virtual unsigned int binaryLowest() const { return 0; }

    virtual unsigned int binaryHighest() const { return 8191; }

    virtual double lowest(unsigned int chan) const { return -10.0; }

    virtual double highest(unsigned int chan) const { return 10.0; }

    /**
    * Resolution is expressed in bits / MU
    */
    virtual double resolution(unsigned int chan) const { return 8192/20.0; }

    virtual unsigned int nbOfChannels() const {return 8; }
};

  
#endif
