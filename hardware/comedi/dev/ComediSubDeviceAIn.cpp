/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:39 CET 2006  ComediSubDeviceAIn.cxx 

                        ComediSubDeviceAIn.cxx -  description
                           -------------------
    begin                : Wed January 18 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@mech.kuleuven.be
 
 ***************************************************************************
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
 
 

#include "ComediSubDeviceAIn.hpp"
#include "comedi_internal.h"
#include <rtt/os/fosi.h>

namespace OCL
{
  
  

    ComediSubDeviceAIn::ComediSubDeviceAIn( ComediDevice* cd, const std::string& name, unsigned int subdevice /*=0*/)
        : RTT::AnalogInInterface( name ),
        myCard( cd ), _subDevice( subdevice ),
        _sd_range(0), _aref(0), channels(0), rrange(0),
        max(0), min(0)
    {
        init();
    }

    ComediSubDeviceAIn::ComediSubDeviceAIn( ComediDevice* cd, unsigned int subdevice /*=0*/ )
        : myCard( cd ), _subDevice( subdevice ),
          _sd_range(0), _aref(0), channels(0), rrange(0),
          max(0), min(0)
    {
        init();
    }

    ComediSubDeviceAIn::~ComediSubDeviceAIn()
    {
        delete[] _sd_range;
        delete[] _aref;
        delete[] max;
        delete[] min;
    }

    void ComediSubDeviceAIn::init()
    {
        if ( !myCard) {
            log(Error) << "Error creating ComediSubDeviceAIn: null ComediDevice given." <<endlog();
            return;
        }
        if ( myCard->getSubDeviceType( _subDevice ) != COMEDI_SUBD_AI )
            {
                log(Error) <<  "comedi_get_subdevice_type failed" << endlog();
                myCard = 0;
                return;
            }

        channels = comedi_get_n_channels(myCard->getDevice()->it, _subDevice);

        _sd_range = new unsigned int[channels];
        _aref = new unsigned int[channels];
        max = new double[channels];
        min = new double[channels];
        // Put default range and _aref into every channel
        for (unsigned int i = 0; i < channels ; i++)
            {
                _sd_range[i] = 0;
                _aref[i] = AREF_GROUND;
                highest(i);
                lowest(i);
            }
    }

    void ComediSubDeviceAIn::rangeSet(unsigned int chan, unsigned int range /*=0*/)
    {
        if ( chan < channels )
            {
                _sd_range[chan] = range;
                highest(chan);
                lowest(chan);
            }
        else log(Error) << "Channel does not exist" << endlog();
    }

    void ComediSubDeviceAIn::arefSet(unsigned int chan, unsigned int aref /*=AREF_GROUND*/)
    {
        if ( chan < channels )
            {
                _aref[chan] = aref;
                highest(chan);
                lowest(chan);
            }
        else log(Error) << "Channel does not exist" << endlog();
    }

    int ComediSubDeviceAIn::rawRead( unsigned int chan, unsigned int& value )
    {
        if ( myCard )
            return myCard->read( _subDevice,chan, _sd_range[chan],
                                 _aref[chan], value );
        return -1;
    }

    int ComediSubDeviceAIn::read( unsigned int chan, double& value )
    {
        unsigned int ival = 0;
        if ( myCard && myCard->read( _subDevice,chan, _sd_range[chan],
                                     _aref[chan], ival ) ) {
            value = min[chan] + ival / resolution(chan);
            return 0;
        }
        return -1;
    }

    unsigned int ComediSubDeviceAIn::rawRange() const
    {
        return myCard ? (rrange = myCard->getMaxData(_subDevice)) : (rrange = 0);
    }
        
    double ComediSubDeviceAIn::lowest(unsigned int chan) const
    {
        if (!myCard)
            return 0.0;
        /* Damned: kcomedilib does not know comedi_range structure but
           uses as comedi_krange struct (probably not to enforce
           floating point support in your RT threads?)
        */
#ifdef __KERNEL__
        // See file:/usr/src/comedilib/doc/html/x3563.html#REF-TYPE-COMEDI-KRANGE
        comedi_krange range;
        comedi_get_krange(myCard->getDevice()->it, _subDevice, chan, 
                          _sd_range[chan], &range);
        return (min[chan] = (double) range.min / 1000000.);
#else
#ifdef OROPKG_OS_LXRT
        //#define __KERNEL__
        comedi_krange range;
        comedi_get_krange(myCard->getDevice()->it, _subDevice, chan, 
                          _sd_range[chan], &range);
        return (min[chan] = (double) range.min / 1000000.);
#else // Userspace
        comedi_range * range_p;
        if ((range_p = comedi_get_range(myCard->getDevice()->it, 
                                        _subDevice, chan, 
                                        _sd_range[chan])) != 0)
            {
                return (min[chan] = range_p->min);
            }
        else
            {
                log(Error) << "Error getting comedi_range struct for channel " << chan << endlog();
                return -1.0;
            }
#endif // Userspace
#endif // __KERNEL__
    }

    double ComediSubDeviceAIn::highest(unsigned int chan) const
    {
        if (!myCard)
            return 0.0;
        /* Damned: kcomedilib does not know comedi_range structure but
           uses as comedi_krange struct (probably not to enforce
           floating point support in your RT threads?)
        */
#ifdef __KERNEL__
        // See file:/usr/src/comedilib/doc/html/x3563.html#REF-TYPE-COMEDI-KRANGE
        comedi_krange range;
        comedi_get_krange(myCard->getDevice()->it, _subDevice, chan, 
                          _sd_range[chan], &range);
        return (max[chan] = (double) range.max / 1000000.);
#else
#ifdef OROPKG_OS_LXRT
        //#define __KERNEL__
        comedi_krange range;
        comedi_get_krange(myCard->getDevice()->it, _subDevice, chan, 
                          _sd_range[chan], &range);
        return (max[chan] = (double) range.max / 1000000.);
#else // Userspace
        comedi_range * range_p;
        if ((range_p = comedi_get_range(myCard->getDevice()->it, 
                                        _subDevice, chan, 
                                        _sd_range[chan])) != 0)
            {
                return (max[chan] = range_p->max);
            }
        else
            {
                log(Error) << "Error getting comedi_range struct for channel " << chan << endlog();
                return -1.0;
            }
#endif // Userspace
#endif // __KERNEL__
    }

    double ComediSubDeviceAIn::resolution(unsigned int chan) const
    {
        if (!myCard)
            return 0.0;
        return rrange / ( max[chan] - min[chan] );
    }

    unsigned int ComediSubDeviceAIn::nbOfChannels() const
    {
        return channels;
    }
}
