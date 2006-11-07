#include "IOComponent.hpp"

using namespace Orocos;

namespace OCL
{

    void IOComponent::createAPI()
    {
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
