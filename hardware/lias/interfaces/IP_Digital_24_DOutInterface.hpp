#ifndef IP_DIGITAL_24_DOUTINTERFACE
#define IP_DIGITAL_24_DOUTINTERFACE



#include <device_interface/DigitalOutInterface.hpp>


class IP_Digital_24_DOutInterface : public ORO_DeviceInterface::DigitalOutInterface
{
public:
    IP_Digital_24_DOutInterface(const std::string& name) : ORO_DeviceInterface::DigitalOutInterface(name) {};
    IP_Digital_24_DOutInterface() {};
    virtual ~IP_Digital_24_DOutInterface() {};

    virtual void switchOn( unsigned int n );

    virtual void switchOff( unsigned int n );

    virtual void setBit( unsigned int bit, bool value );

    virtual void setSequence(unsigned int start_bit, unsigned int stop_bit, unsigned int value);

    virtual bool checkBit(unsigned int n) const;

    virtual unsigned int checkSequence( unsigned int start_bit, unsigned int stop_bit ) const;

    virtual unsigned int nbOfOutputs() const { return 24; }
};


#endif
