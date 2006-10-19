#include <pkgconf/system.h>

#if defined (OROPKG_OS_LXRT)
#include <rtt/PeriodicActivity.hpp>

#include "IP_Encoder_6_EncInterface.hpp"
#include "drivers/LiAS_lxrt_user.h"

using namespace RTT;

IP_Encoder_6_Task::IP_Encoder_6_Task( Seconds period )
    : PeriodicActivity(0, period )
    , _virtual_encoder_1("encoder1"), _virtual_encoder_2("encoder2"), _virtual_encoder_3("encoder3")
    , _virtual_encoder_4("encoder4"), _virtual_encoder_5("encoder5"), _virtual_encoder_6("encoder6")
{
    for (unsigned int i = 0; i < 6; i++)
    {
        _virtualEncoder[i] = 0;
        _prevEncoderValue[i] = 0;
    }

    _virtual_encoder_1.Set(_virtualEncoder[0]);
    _virtual_encoder_2.Set(_virtualEncoder[1]);
    _virtual_encoder_3.Set(_virtualEncoder[2]);
    _virtual_encoder_4.Set(_virtualEncoder[3]);
    _virtual_encoder_5.Set(_virtualEncoder[4]);
    _virtual_encoder_6.Set(_virtualEncoder[5]);
}



bool IP_Encoder_6_Task::initialize()
{
    //  Initialise the encoder values
    for (unsigned int i = 0; i< 6; i++)
    {
        // Get the current value of the encoder
        _prevEncoderValue[i] = IP_Encoder_6_get_counter_channel( i+1 );
        
        // and just to check that IF it is 0xffff, it is supposed to be:
        if (_prevEncoderValue[i] == 0xffff) _prevEncoderValue[i] = IP_Encoder_6_get_counter_channel( i+1 );
    }
    
    return true;
}



void IP_Encoder_6_Task::step()
{
#if TEST_ENCODERS
    static int counter(0);
    counter++;
    if (counter == 100)
    {
      printf("Encoder: [%ld %ld %ld %ld %ld %ld ] ticks\n", _virtualEncoder[0], _virtualEncoder[1], _virtualEncoder[2],
                                                            _virtualEncoder[3], _virtualEncoder[4], _virtualEncoder[5]);
      counter = 0;
    }
#endif
    for (unsigned int i = 0; i< 6; i++)
    {
        // Get the current value of the encoder ( 1..6 )
        unsigned short curPos = IP_Encoder_6_get_counter_channel( i+1 );

        // See if is an (unlikely) 65536 (0xffff), which could be an error
        if (curPos == 0xffff) curPos = IP_Encoder_6_get_counter_channel( i+1 );

        // Calculate the difference
        int delta = (int) curPos - (int)_prevEncoderValue[i];

        // Take into account that the encoder could have overflowed/underflowed
        // (For reasons not to use the registers for this, see IP-Encoder6.h)
        if (delta >  32768) delta -= 65536;
        if (delta < -32768) delta += 65536;
    
        // remember the current position for next time
        _prevEncoderValue[i] = curPos;
        // update
        _virtualEncoder[i] += delta;
    }
    
    _virtual_encoder_1.Set(_virtualEncoder[0]);
    _virtual_encoder_2.Set(_virtualEncoder[1]);
    _virtual_encoder_3.Set(_virtualEncoder[2]);
    _virtual_encoder_4.Set(_virtualEncoder[3]);
    _virtual_encoder_5.Set(_virtualEncoder[4]);
    _virtual_encoder_6.Set(_virtualEncoder[5]);
}



void IP_Encoder_6_Task::positionSet(unsigned int encoderNr, int pos)
{
    // Reset the current value
    _virtualEncoder[encoderNr] = pos;
    // Remember the current position of the real encoder
    _prevEncoderValue[encoderNr] = IP_Encoder_6_get_counter_channel( encoderNr + 1 );

    switch ( encoderNr )
    {
        case 0 : _virtual_encoder_1.Set(_virtualEncoder[0]); break;
        case 1 : _virtual_encoder_2.Set(_virtualEncoder[1]); break;
        case 2 : _virtual_encoder_3.Set(_virtualEncoder[2]); break;
        case 3 : _virtual_encoder_4.Set(_virtualEncoder[3]); break;
        case 4 : _virtual_encoder_5.Set(_virtualEncoder[4]); break;
        case 5 : _virtual_encoder_6.Set(_virtualEncoder[5]); break;
    }
}


void IP_Encoder_6_Task::turnSet(unsigned int encoderNr, int turn)
{
    // We don't do turns!
}



const int IP_Encoder_6_Task::positionGet(unsigned int encoderNr) const
{
    switch ( encoderNr )
    {
        case 0 : return _virtual_encoder_1.Get();
        case 1 : return _virtual_encoder_2.Get();
        case 2 : return _virtual_encoder_3.Get();
        case 3 : return _virtual_encoder_4.Get();
        case 4 : return _virtual_encoder_5.Get();
        case 5 : return _virtual_encoder_6.Get();
    }
    return 0;
};



const int IP_Encoder_6_Task::turnGet(unsigned int encoderNr) const
{
    // We don't do turns!
    return 0;
};




IP_Encoder_6_EncInterface::IP_Encoder_6_EncInterface( IP_Encoder_6_Task& IP_encoder_task, const unsigned int& encoder_nr, const std::string& name )
    : EncoderInterface( name )
{
    _encoder_nr = encoder_nr;
    _encoder_task = &IP_encoder_task;
}



IP_Encoder_6_EncInterface::IP_Encoder_6_EncInterface( IP_Encoder_6_Task& IP_encoder_task, const unsigned int& encoder_nr )
{
    _encoder_nr = encoder_nr;
    _encoder_task = &IP_encoder_task;
}

#endif
