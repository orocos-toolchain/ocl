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

#ifndef _COMBINED_DIGITAL_OUT_
#define _COMBINED_DIGITAL_OUT_

#include <rtt/dev/DigitalOutput.hpp>
#include <vector>

namespace RTT
{
    enum combinetype {OR, AND};

    class CombinedDigitalOutInterface: public DigitalOutInterface
    {
        private:
            std::vector<bool>                 _channels;
            RTT::DigitalOutput*  _digitalout;
            enum combinetype                  _combine;
            
            void refresh();


        public:
            CombinedDigitalOutInterface (const std::string& name, RTT::DigitalOutput* digitalout, unsigned int num_channels, combinetype type);
            virtual ~CombinedDigitalOutInterface();

            virtual void switchOn( unsigned int n );
            virtual void switchOff( unsigned int n );
            virtual void setBit( unsigned int bit, bool value );
            virtual void setSequence(unsigned int start_bit, unsigned int stop_bit, unsigned int value);
            virtual bool checkBit(unsigned int n) const;
            virtual unsigned int checkSequence( unsigned int start_bit, unsigned int stop_bit ) const;
            virtual unsigned int nbOfOutputs() const;
    };

} // end namespace
#endif
