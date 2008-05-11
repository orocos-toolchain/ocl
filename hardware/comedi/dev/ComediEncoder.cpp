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
    ComediEncoder::ComediEncoder(ComediDevice * cd, unsigned int subd, 
                                 unsigned int encNr, const std::string& name)
        : EncoderInterface(name),
          _myCard(cd), _subDevice(subd), _channel(encNr),
          _turn(0), _upcounting(true)
    {
        init();
    }

    ComediEncoder::ComediEncoder(ComediDevice * cd, unsigned int subd, 
                                 unsigned int encNr)
        :  _myCard(cd), _subDevice(subd), _channel(encNr),
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
        // Check how many counters this subdevice actually has
        unsigned int nchan = comedi_get_n_channels(_myCard->getDevice()->it,_subDevice);
        if ( nchan <= _channel )
            {
                log(Error) << "Comedi Counter : Only " << nchan 
                           << " channels on this counter subdevice" << endlog();
                _myCard = 0;
                return;
            }
        /* Configure the counter subdevice
           Configure the GPCT for use as an encoder 
           Uses the functions from comedi_common.c
        */
        int retval = reset_counter(_myCard->getDevice()->it, _subDevice, _channel);
        /* set initial counter value by writing to channel 0 */
        unsigned int initial_value = 0;
        retval = comedi_data_write(_myCard->getDevice()->it, _subDevice, 0, 0, 0, initial_value);

        int counter_mode = (NI_GPCT_COUNTING_MODE_QUADRATURE_X4_BITS |
                        NI_GPCT_COUNTING_DIRECTION_HW_UP_DOWN_BITS);

        int retval1 = set_counter_mode(_myCard->getDevice()->it, _subDevice, counter_mode, _channel);
        int retval2 = arm(_myCard->getDevice()->it, _subDevice, NI_GPCT_ARM_IMMEDIATE, _channel);
        if(retval1 < 0 || retval1 < 0) {
            log(Error) << "Comedi Counter : Instruction to configure counter -> encoder failed (ret:"<<retval1<<","<<retval2<<")" << endlog();
            _myCard = 0;
        } else
            log(Info) << "Comedi Counter : configured as encoder now" << endlog();

        _resolution = comedi_get_maxdata(_myCard->getDevice()->it,_subDevice, 
                                               _channel);
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
        comedi_data_write(_myCard->getDevice()->it, _subDevice,
                          _channel, 0, 0, (lsampl_t) p);
    }

    void ComediEncoder::turnSet(int t){ _turn = t;}
    int ComediEncoder::turnGet() const { return _turn;}

    int ComediEncoder::positionGet() const
    {
        if (!_myCard)
            return 0;

        typedef unsigned int Data;
        //int pos;
        lsampl_t pos[20];
        int ret=comedi_data_read(_myCard->getDevice()->it,_subDevice,_channel,0,0,pos);
        //int ret=comedi_data_read(_myCard->getDevice(),_subDevice,_channel,0,0,(unsigned int *)&pos);
        if(ret<0){
            log(Error) << "Comedi Counter : reading encoder failed, ret = " << ret << endlog();
        }
        // Other possibility for reading the data (with instruction)
        /*    
              comedi_insn insn;
              Data readdata; // local data
              insn.insn=INSN_READ;
              insn.n=1; // Irrelevant for config
              insn.data=&readdata;
              insn.subdev=_subDevice;
              insn.chanspec=CR_PACK(_channel,0,0);
              int ret=comedi_do_insn(_myCard->getDevice(),&insn);
              if(ret<0)
              {
                 log(Error) << "Comedi Counter : reading encoder failed, ret = " << ret << endlog();
              }
              pos = readdata;
        */
        return pos[0];
    } 

    int ComediEncoder::resolution() const {return _resolution;}
  
    bool ComediEncoder::upcounting() const {return _upcounting;}

}
