#include <rtt/RTT.hpp>

#if defined (OROPKG_OS_LXRT)


#include "IP_FastDAC_AOutInterface.hpp"



IP_FastDAC_AOutInterface::IP_FastDAC_AOutInterface(const std::string& name) : RTT::AnalogOutInterface<unsigned int>(name)
{
    IP_FastDAC_set_gain_of_group (1, RANGE_10_V);
    IP_FastDAC_set_gain_of_group (2, RANGE_10_V);
    IP_FastDAC_set_gain_of_group (3, RANGE_10_V);
    IP_FastDAC_set_gain_of_group (4, RANGE_10_V);

    IP_FastDAC_enable_dac ( );
};

IP_FastDAC_AOutInterface::IP_FastDAC_AOutInterface() : RTT::AnalogOutInterface<unsigned int>()
{
    IP_FastDAC_set_gain_of_group (1, RANGE_10_V);
    IP_FastDAC_set_gain_of_group (2, RANGE_10_V);
    IP_FastDAC_set_gain_of_group (3, RANGE_10_V);
    IP_FastDAC_set_gain_of_group (4, RANGE_10_V);

    IP_FastDAC_enable_dac ( );
};

#endif
