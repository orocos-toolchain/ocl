#include <pkgconf/system.h>

#if defined (OROPKG_OS_LXRT)


#include "IP_Digital_24_DOutInterface.hpp"
#include "../drivers/LiAS_lxrt_user.h"

void IP_Digital_24_DOutInterface::switchOn( unsigned int n )
{
    IP_Digital_24_set_bit_of_channel( n + 1 );
}


void IP_Digital_24_DOutInterface::switchOff( unsigned int n )
{
    IP_Digital_24_clear_bit_of_channel( n + 1 );
}


void IP_Digital_24_DOutInterface::setBit( unsigned int bit, bool value )
{
    if (value) IP_Digital_24_set_bit_of_channel( bit + 1 );
    else IP_Digital_24_clear_bit_of_channel( bit + 1 );
}


void IP_Digital_24_DOutInterface::setSequence(unsigned int start_bit, unsigned int stop_bit, unsigned int value)
{
    unsigned int temp;
    
    for(unsigned int i = start_bit; i < stop_bit; i++)
    {
        temp = (1 << i);
        temp &= value;
        if ( temp ) IP_Digital_24_set_bit_of_channel( i + 1 );
        else IP_Digital_24_clear_bit_of_channel( i + 1 );
    }
}


bool IP_Digital_24_DOutInterface::checkBit( unsigned int bit ) const
{
    return IP_Digital_24_get_bit_of_channel( bit + 1 );
}


unsigned int IP_Digital_24_DOutInterface::checkSequence( unsigned int start_bit, unsigned int stop_bit ) const
{
    unsigned int value = 0;

    for (unsigned int i = start_bit; i < stop_bit; i++)
        value |= (IP_Digital_24_get_bit_of_channel( i + 1 ) << i );

    return value;
}

#endif
