#ifndef IP_OPTOINPUT_DININTERFACE_HPP
#define IP_OPTOINPUT_DININTERFACE_HPP


#include <rtt/dev/DigitalInInterface.hpp>


class IP_OptoInput_DInInterface : public RTT::DigitalInInterface
{
public:
    IP_OptoInput_DInInterface(const std::string& name) : RTT::DigitalInInterface(name) {};
    IP_OptoInput_DInInterface() {};
    virtual ~IP_OptoInput_DInInterface() {};

    virtual bool isOn( unsigned int bit = 0) const;

    virtual bool isOff( unsigned int bit = 0) const;

    virtual bool readBit( unsigned int bit = 0) const;

    virtual unsigned int readSequence(unsigned int start_bit, unsigned int stop_bit) const;

    virtual unsigned int nbOfInputs() const { return 16; }
};


#endif
