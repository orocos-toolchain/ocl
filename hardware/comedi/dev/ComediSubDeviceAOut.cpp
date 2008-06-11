/***************************************************************************
  tag: Peter Soetens  Thu Oct 10 16:22:44 CEST 2002  ComediSubDeviceAOut.hpp 

                        ComediSubDeviceAOut.hpp -  description
                           -------------------
    begin                : Thu October 10 2002
    copyright            : (C) 2002 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

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
/* Klaas Gadeyne August 2003: implemented some non/badly implemented
   stuff.
*/

#include "ComediSubDeviceAOut.hpp"
#include "ComediDevice.hpp"
#include <rtt/os/fosi.h>
#include "comedi_internal.h"

namespace OCL
{

    ComediSubDeviceAOut::ComediSubDeviceAOut( ComediDevice* cao, const std::string& name, 
                                              unsigned int subdevice /*=1*/ )
        : AnalogOutInterface( name ),
          myCard( cao ), _subDevice( subdevice ),
          _sd_range(0), _aref(0), channels(0), rrange(0),
          max(0), min(0)
    {
        init();
    }

    ComediSubDeviceAOut::ComediSubDeviceAOut( ComediDevice* cao, unsigned int subdevice /*=1*/ )
        : myCard( cao ), _subDevice( subdevice ),
          _sd_range(0), _aref(0), channels(0), rrange(0),
          max(0), min(0)
    {
        init();
    }

    ComediSubDeviceAOut::~ComediSubDeviceAOut()
    {
        delete[] _sd_range;
        delete[] _aref;
        delete[] max;
        delete[] min;
    }

    void ComediSubDeviceAOut::init()
    {
        if ( !myCard) {
            log(Error) << "Error creating ComediSubDeviceAOut: null ComediDevice given." <<endlog();
            return;
        }
        if ( myCard->getSubDeviceType( _subDevice ) != COMEDI_SUBD_AO )
            {
                log(Error) <<  "comedi_get_subdevice_type failed" <<endlog();
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
        // Load rrange value.
        rawRange();
    }

    void ComediSubDeviceAOut::rangeSet(unsigned int chan, unsigned int range /*=0*/)
    {
        if ( chan < channels )
            {
                _sd_range[chan] = range;
                highest(chan);
                lowest(chan);
            }
        else log(Error) << "Channel does not exist" << endlog();
    }

    void ComediSubDeviceAOut::arefSet(unsigned int chan, unsigned int aref /*=AREF_GROUND*/)
    {
        if ( chan < channels )
            {
                _aref[chan] = aref;
                highest(chan);
                lowest(chan);
            }
        else log(Error) << "Channel does not exist" << endlog();
    }

    int ComediSubDeviceAOut::rawWrite( unsigned int chan, int value )
    {
        if (value < 0) value = 0;
        if (value > int(rrange)) value = rrange;
        if ( myCard )
            return myCard->write( _subDevice, chan, _sd_range[chan], 
                                  _aref[chan], (unsigned int)(value) );
        return -1;
    }

    int ComediSubDeviceAOut::rawRead( unsigned int chan, int& value)
    {
        if ( myCard ) {
            unsigned int uval;
            int ret = myCard->read( _subDevice, chan, _sd_range[chan], 
                          _aref[chan], uval );
            value = uval;
            return ret;
        }
        return -1;
    }

    int ComediSubDeviceAOut::write( unsigned int chan, double dvalue )
    {
        //limit dvalue to min and max values
        if(dvalue<min[chan]) dvalue=min[chan];
        if(dvalue>max[chan]) dvalue=max[chan];
        unsigned int value = (unsigned int)((dvalue - min[chan]) * resolution(chan));
        if ( myCard )
            return myCard->write( _subDevice, chan, _sd_range[chan], 
                                  _aref[chan], value );
        return -1;
    }

    int ComediSubDeviceAOut::read( unsigned int chan, double& dvalue )
    {
        unsigned int ival = 0;
        if ( myCard && myCard->read( _subDevice,chan, _sd_range[chan],
                                     _aref[chan], ival ) == 0 ) {
            dvalue = min[chan] + ival / resolution(chan);
            return 0;
        }
        return -1;
    }


    unsigned int ComediSubDeviceAOut::rawRange() const
    {
        return myCard ? (rrange = myCard->getMaxData(_subDevice)) : (rrange = 0);
    }

    double ComediSubDeviceAOut::lowest(unsigned int chan) const
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

    double ComediSubDeviceAOut::highest(unsigned int chan) const
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

    double ComediSubDeviceAOut::resolution(unsigned int chan) const
    {
        if (!myCard)
            return 0.0;
        return rrange / ( max[chan] - min[chan] );
    }

    unsigned int ComediSubDeviceAOut::nbOfChannels() const
    {
        return channels;
    }

};

