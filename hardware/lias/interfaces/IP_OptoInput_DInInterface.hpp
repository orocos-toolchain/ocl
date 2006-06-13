#ifndef IP_OPTOINPUT_DININTERFACE_HPP
#define IP_OPTOINPUT_DININTERFACE_HPP


#include <device_interface/DigitalInInterface.hpp>


class IP_OptoInput_DInInterface : public ORO_DeviceInterface::DigitalInInterface
{
public:
    IP_OptoInput_DInInterface(const std::string& name) : ORO_DeviceInterface::DigitalInInterface(name) {};
    IP_OptoInput_DInInterface() {};
    virtual ~IP_OptoInput_DInInterface() {};

    virtual bool isOn( unsigned int bit = 0) const;

    virtual bool isOff( unsigned int bit = 0) const;

    virtual bool readBit( unsigned int bit = 0) const;

    virtual unsigned int readSequence(unsigned int start_bit, unsigned int stop_bit) const;

    virtual unsigned int nbOfInputs() const { return 16; }
};


#endif
