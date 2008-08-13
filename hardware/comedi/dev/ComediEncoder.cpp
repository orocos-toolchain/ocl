// Copyright (C) 2003,2007 Klaas Gadeyne
//
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

#include "ComediEncoder.hpp"
#include <rtt/Logger.hpp>

#include "comedi_internal.h"
#include "comedi_common.h"
#include "comedi.h"

namespace OCL
{
    ComediEncoder::ComediEncoder(ComediDevice * cd, unsigned int subd, const std::string& name)
        : EncoderInterface(name),
          _myCard(cd), _subDevice(subd),
          _turn(0), _upcounting(true)
    {
        init();
    }

    ComediEncoder::ComediEncoder(ComediDevice * cd, unsigned int subd)
        :  _myCard(cd), _subDevice(subd),
           _turn(0), _upcounting(true)
    {
        init();
    }

    void ComediEncoder::init()
    {
        Logger::In in("ComediEncoder");
        if (!_myCard) {
            log(Error) << "Error creating ComediEncoder: null ComediDevice given." <<endlog();
            return;
        }
        log(Info) << "Creating ComediEncoder\n" << endlog();
        // Check if subd is counter...
        if ( _myCard->getSubDeviceType( _subDevice ) != COMEDI_SUBD_COUNTER )
            {
                log(Error) << "Comedi Counter : subdev is not a counter, type = "
                           << _myCard->getSubDeviceType(_subDevice) << endlog();
                _myCard = 0;
                return;
            }

        /* Configure the counter subdevice
           Configure the GPCT for use as an encoder
           Uses the functions from comedi_common.c
        */
        int retval = reset_counter(_myCard->getDevice()->it, _subDevice);
        /* set initial counter value by writing to channel 0 */
        unsigned int initial_value = 0;
        retval = comedi_data_write(_myCard->getDevice()->it, _subDevice, 0, 0, 0, initial_value);

        int counter_mode = (NI_GPCT_COUNTING_MODE_QUADRATURE_X4_BITS |
                        NI_GPCT_COUNTING_DIRECTION_HW_UP_DOWN_BITS);

        int retval1 = set_counter_mode(_myCard->getDevice()->it, _subDevice, counter_mode);
        int retval2 = arm(_myCard->getDevice()->it, _subDevice, NI_GPCT_ARM_IMMEDIATE);
        if(retval1 < 0 || retval2 < 0) {
            log(Error) << "Comedi Counter : Instruction to configure counter -> encoder failed (ret:"<<retval1<<","<<retval2<<")" << endlog();
            _myCard = 0;
        } else
            log(Info) << "Comedi Counter : configured as encoder now" << endlog();

        _resolution = comedi_get_maxdata(_myCard->getDevice()->it,_subDevice, 0);
        if ( _resolution == 0) {
            log(Error) << "Comedi Counter : Could not retrieve encoder resolution !"<<endlog();
        }
    }

    ComediEncoder::~ComediEncoder(){}

    void ComediEncoder::positionSet(int p)
    {
        if (!_myCard)
            return;
        //int can be negative, by casting the int to lsampl_t(unsigned int)
        // we write the right value to the encoderdevice
        int retval;
        /* set initial counter value by writing to channel 0 */
        retval = comedi_data_write(_myCard->getDevice()->it, _subDevice, 0, 0, 0, (lsampl_t)p);
        /* set "load a" register to initial_value by writing to channel 1 */
        retval = comedi_data_write(_myCard->getDevice()->it, _subDevice, 1, 0, 0, (lsampl_t)p);
        /* set "load b" register to initial_value by writing to channel 2 */
        retval = comedi_data_write(_myCard->getDevice()->it, _subDevice, 2, 0, 0, (lsampl_t)p);
    }

    void ComediEncoder::turnSet(int t){ _turn = t;}
    int ComediEncoder::turnGet() const { return _turn;}

    int ComediEncoder::positionGet() const
    {
        if (!_myCard)
            return 0;

        lsampl_t pos;
        int ret=comedi_data_read(_myCard->getDevice()->it,_subDevice,0,0,0,&pos);

        if(ret<0){
            log(Error) << "Comedi Counter : reading encoder failed, ret = " << ret << endlog();
        }
        return pos;
    }

    int ComediEncoder::resolution() const {return _resolution;}

    bool ComediEncoder::upcounting() const {return _upcounting;}

}
