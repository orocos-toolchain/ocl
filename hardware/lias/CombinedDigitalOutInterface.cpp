// $Id: gaussian_double.h,v 2.26 2005/05/26 12:07:29 wmeeusse Exp $
// Copyright (C) 2005 Wim Meeussen <first dot last at mech dot kuleuven dot be>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  

#include <rtt/RTT.hpp>

#if defined (OROPKG_OS_LXRT)


//#include <lias/LiASHardware.hpp>
#include "CombinedDigitalOutInterface.hpp"
#include <assert.h>

using namespace RTT;




std::vector<bool>  channels;

CombinedDigitalOutInterface::CombinedDigitalOutInterface( const std::string& name, RTT::DigitalOutput* digitalout
                                                        , unsigned int num_channels, combinetype type)
   : DigitalOutInterface(name), _channels(num_channels), _digitalout(digitalout), _combine(type)
{}



CombinedDigitalOutInterface::~CombinedDigitalOutInterface()
{}



void CombinedDigitalOutInterface::switchOn( unsigned int n )
{
    assert(n>=0 && n<_channels.size());
    _channels[n] = true;

    refresh();
}



void CombinedDigitalOutInterface::switchOff( unsigned int n )
{
    assert(n>=0 && n<_channels.size());
    _channels[n] = false;

    refresh();
}



void CombinedDigitalOutInterface::setBit( unsigned int bit, bool value )
{
    assert(bit>=0 && bit<_channels.size());
    _channels[bit] = value;

    refresh();
}



void CombinedDigitalOutInterface::setSequence(unsigned int start_bit, unsigned int stop_bit, unsigned int value)
{
    assert(start_bit>=0 && start_bit<_channels.size());
    assert(stop_bit>=0  && stop_bit<_channels.size());
    assert(start_bit <= stop_bit);

    for (unsigned int i=start_bit; i<=stop_bit; i++)
        _channels[i] = value;

    refresh();
}



bool CombinedDigitalOutInterface::checkBit(unsigned int bit) const
{
    assert(bit>=0 && bit<_channels.size());
    return _channels[bit];
} 



unsigned int CombinedDigitalOutInterface::checkSequence( unsigned int start_bit, unsigned int stop_bit ) const
{
    assert(start_bit>=0 && start_bit<_channels.size());
    assert(stop_bit>=0  && stop_bit<_channels.size());
    assert(start_bit <= stop_bit);

    unsigned int result = 0;
    for (unsigned int i=start_bit; i<=stop_bit; i++)
        if (_channels[i])
            result++;

    return result;
}



unsigned int CombinedDigitalOutInterface::nbOfOutputs() const
{
    return _channels.size();
}



void CombinedDigitalOutInterface::refresh()
{
    switch (_combine)
    {
        bool enable;
        
        case OR:
            enable = false;
            for (unsigned int i=0; i<_channels.size(); i++)
                if (_channels[i]) enable = true;

            if (enable)
                _digitalout->switchOn();
            else
                _digitalout->switchOff();
            break;

        case AND:
            enable = true;
            for (unsigned int i=0; i<_channels.size(); i++)
                if (!_channels[i]) enable = false;

            if (enable)
                _digitalout->switchOn();
            else
                _digitalout->switchOff();
            break;
    }
}

#endif
