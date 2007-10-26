// Copyright (C) 2003 Klaas Gadeyne

/***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#include "ComediSubDeviceDOut.hpp"
#include "rtt/os/fosi.h"

#include <rtt/RTT.hpp>

#include "comedi_internal.h"

namespace RTT
{
    ComediSubDeviceDOut::ComediSubDeviceDOut( ComediDevice* cd, const std::string& name, unsigned int subdevice)
        : DigitalOutInterface( name ),
          myCard( cd ), subDevice( subdevice ), channels(0)
    {
        init();
    }

    ComediSubDeviceDOut::ComediSubDeviceDOut( ComediDevice* cd, unsigned int subdevice )
        : myCard( cd ), subDevice( subdevice ), channels(0)
    {
        init();
    }

    void ComediSubDeviceDOut::init()
    {
        if (!myCard) {
            log(Error) << "Error creating ComediSubDeviceDOut: null ComediDevice given." <<endlog();
            return;
        }
        if ( ( myCard->getSubDeviceType( subDevice ) != COMEDI_SUBD_DO ) &&
             ( myCard->getSubDeviceType( subDevice ) != COMEDI_SUBD_DIO) )
            {
                Logger::In in("ComediSubDeviceDOut");
                log(Error) << "SubDevice "<< subDevice <<" is not a digital output:";
                log() << "type = " << myCard->getSubDeviceType( subDevice ) << endlog();
                // channels remains '0'.
                return;
            }

        channels = comedi_get_n_channels(myCard->getDevice()->it, subDevice);
        
        // Only for DIO
        if ( ( myCard->getSubDeviceType( subDevice ) != COMEDI_SUBD_DIO) ) {
            log(Info) << "Setting all dio on subdevice "<<subDevice<<" to output type." << endlog();
        
            for (unsigned int i=0; i<channels; ++i)
                comedi_dio_config(myCard->getDevice()->it, subDevice, i, COMEDI_OUTPUT);
        }

    }

    void ComediSubDeviceDOut::switchOn( unsigned int bit)
    {
        if (bit < channels)
            comedi_dio_write( myCard->getDevice()->it,subDevice,bit,1);
    }

    void ComediSubDeviceDOut::switchOff( unsigned int bit)
    {
        if (bit < channels)
            comedi_dio_write( myCard->getDevice()->it,subDevice,bit,0);
    }

    void ComediSubDeviceDOut::setBit( unsigned int bit, bool value)
    {
        if (bit < channels)
            {
                if (value == true)
                    comedi_dio_write( myCard->getDevice()->it,subDevice,bit,1);
                else
                    comedi_dio_write( myCard->getDevice()->it,subDevice,bit,0);
            }
    }


    void ComediSubDeviceDOut::setSequence(unsigned int start_bit, unsigned int stop_bit, unsigned int value)
    {
        if ((start_bit > stop_bit) || (stop_bit >= channels))
            {
                Logger::In in("ComediSubDeviceDOut");
                log(Error)<< "start_bit should be less than stop_bit) and stopbit can be bigger than the number of channels" << endlog();
                return;
            }
        else
            {
                unsigned int write_mask = 0;
                // FIXME: Can somebody check this cumbersome line please?
                for (unsigned int i = start_bit; i <= stop_bit ; i++) 
                    {
                        write_mask = write_mask | (0x1 << i);
                    }
                // Shift Value startbits to the left
                value = value << start_bit;
                comedi_dio_bitfield(myCard->getDevice()->it, subDevice, write_mask, &value);
            }
    }

    bool ComediSubDeviceDOut::checkBit( unsigned int bit) const
    {
        unsigned int value = 0;
        // read DO or DIO
        comedi_dio_read(myCard->getDevice()->it, subDevice, bit, &value);
        return value == 1;
    }

    unsigned int ComediSubDeviceDOut::checkSequence (unsigned int start_bit, unsigned int stop_bit) const
    {
        Logger::In in("ComediSubDeviceDOut");
        log(Error) << "CheckSequence not implemented, returning 0x0"<< endlog();
        return 0;
    }

    unsigned int ComediSubDeviceDOut::nbOfOutputs() const
    {
        return channels;
    }


}
