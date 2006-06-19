
#include <pkgconf/system.h>

#if defined (OROPKG_OS_LXRT)


#include "IP_OptoInput_DInInterface.hpp"
#include "../drivers/LiAS_lxrt_user.h"


bool IP_OptoInput_DInInterface::isOn( unsigned int bit ) const
{
    unsigned int mask = (1 << bit);
    mask &= IP_OptoInput_Read_IDR();
    return (mask >> bit);
}

bool IP_OptoInput_DInInterface::isOff( unsigned int bit ) const
{
    unsigned int mask = (1 << bit);
    mask &= IP_OptoInput_Read_IDR();
    return !(mask >> bit);
}

bool IP_OptoInput_DInInterface::readBit( unsigned int bit ) const
{
    unsigned int mask = (1 << bit);
    mask &= IP_OptoInput_Read_IDR();
    return (mask >> bit);
}

unsigned int IP_OptoInput_DInInterface::readSequence(unsigned int start_bit, unsigned int stop_bit) const
{
    unsigned int mask = 0;
    for (unsigned int i = start_bit; i < stop_bit; i++)
    {
        mask |= ( 1 << i);
    }

    mask &= IP_OptoInput_Read_IDR();
    
    return mask;
}


#endif
