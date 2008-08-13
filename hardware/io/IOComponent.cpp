#include "IOComponent.hpp"


#include "ocl/ComponentLoader.hpp"
ORO_CREATE_COMPONENT( OCL::IOComponent )

using namespace Orocos;

namespace OCL
{

    void IOComponent::createAPI()
    {
        // Adding a single channel:
        this->methods()->addMethod( method( "addAnalogInput", &IOComponent::addAnalogInput, this),
                                    "Add an analog input as data port",
                                    "PortName", "The name of the port to create.",
                                    "Inputname", "The name of the AnalogInInterface device to use.",
                                    "Channel", "The channel of the device to use.");
        this->methods()->addMethod( method( "addAnalogOutput", &IOComponent::addAnalogOutput, this),
                                    "Add an analog input as data port",
                                    "PortName", "The name of the port to create.",
                                    "Outputname", "The name of the AnalogOutInterface device to use.",
                                    "Channel", "The channel of the device to use.");
        this->methods()->addMethod( method( "addDigitalInput", &IOComponent::addDigitalInput, this),
                                    "Add an digital input",
                                    "Name", "The name of the new input.",
                                    "Inputname", "The name of the DigitalInInterface device to use.",
                                    "Channel", "The channel of the device to use.",
                                    "Invert", "Set to true to invert this bit.");
        this->methods()->addMethod( method( "addDigitalOutput", &IOComponent::addDigitalOutput, this),
                                    "Add an digital input",
                                    "Name", "The name of the new output.",
                                    "Outputname", "The name of the DigitalOutInterface device to use.",
                                    "Channel", "The channel of the device to use.",
                                    "Invert", "Set to true to invert this bit.");

        // Add whole device:
        this->methods()->addMethod( method( "addAnalogInputs", &IOComponent::addAnalogInInterface, this),
                                    "Add an AnalogInInterface as an array data port",
                                    "PortName", "The name of the port to create.",
                                    "Inputname", "The name of the AnalogInInterface device to use.");

        this->methods()->addMethod( method( "addAnalogOutputs", &IOComponent::addAnalogOutInterface, this),
                                    "Add an AnalogOutInterface as an array data port",
                                    "PortName", "The name of the port to create.",
                                    "Outputname", "The name of the AnalogOutInterface device to use.");

        this->methods()->addMethod( method( "addInputChannel", &IOComponent::addInputChannel, this),
                                    "Add an analog input signal into InputValues",
                                    "Pos", "The position of the signal in ChannelValues.",
                                    "InputName", "The name of the AnalogInInterface device to use.",
                                    "Channel", "The channel of the device to use.");
        this->methods()->addMethod( method( "addOutputChannel", &IOComponent::addOutputChannel, this),
                                    "Add an analog input signal into OutputValues",
                                    "Pos", "The position of the signal in ChannelValues.",
                                    "OutputName", "The name of the AnalogOutInterface device to use.",
                                    "Channel", "The channel of the device to use.");

        // Digital/Analog querries
        this->methods()->addMethod( method( "switchOn", &IOComponent::switchOn, this),
                                    "Switch A Digital Output on",
                                    "Name","The Name of the DigitalOutput."
                                    );
        this->methods()->addMethod( method( "isOn", &IOComponent::isOn, this),
                                    "Inspect the status of a Digital Input or Output.",
                                    "Name", "The Name of the Digital Input or Output."
                                    );
        this->methods()->addMethod( method( "switchOff", &IOComponent::switchOff, this),
                                    "Switch A Digital Output off",
                                    "Name","The Name of the DigitalOutput."
                                    );
        this->methods()->addMethod( method( "value", &IOComponent::value, this),
                                    "Inspect the value of an Analog Input or Output.",
                                    "Name", "The Name of the Analog Input or Output."
                                    );
        this->methods()->addMethod( method( "rawValue", &IOComponent::rawValue, this),
                                    "Inspect the raw value of an Analog Input or Output.",
                                    "Name", "The Name of the Analog Input or Output."
                                    );
        this->methods()->addMethod( method( "inputChannels", &IOComponent::getInputChannels, this),
                                    "Get the number of channels this component monitors."
                                    );
        this->methods()->addMethod( method( "outputChannels", &IOComponent::getOutputChannels, this),
                                    "Get the number of channels this component outputs."
                                    );
    }
}
