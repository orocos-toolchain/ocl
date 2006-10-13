// Copyright (C) 2006 FMTC
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  

#include "ComediPulseTrainGenerator.hpp"
#include <rtt/Logger.hpp>

#include "comedi_internal.h"

namespace RTT
{
    typedef unsigned int Data;

    ComediPulseTrainGenerator::ComediPulseTrainGenerator(ComediDevice * cd, unsigned int subd, 
                                                         unsigned int encNr, const std::string& name)
        : _myCard(cd), _subDevice(subd), _channel(encNr),
          _pulse_width(0), _pulse_period(0), _running(false)
    {
        init();
    }

    ComediPulseTrainGenerator::ComediPulseTrainGenerator(ComediDevice * cd, unsigned int subd, 
                                                         unsigned int encNr)
      :  _myCard(cd), _subDevice(subd), _channel(encNr),
	 _pulse_width(0), _pulse_period(0), _running(false)
    {
        init();
    }

    unsigned int ComediPulseTrainGenerator::psecs_to_timebase(psecs picos)
    {
        if (picos < _smallest_step){
            log(Error) << "ComediPTG: Period or Pulsewidth too small for current timebase, picos = " << picos 
		       << ".  Returning ZERO" << endlog();
            return 0;
        }
        if (picos % _smallest_step != 0){
            log(Warning) << "ComediPTG: " << picos << " is not a multiple of " << _smallest_step 
                         << " -> There will be a rounding error" << endlog();
        }
        return picos / _smallest_step;
    }
    
    void ComediPulseTrainGenerator::init()
    {
        log(Info) << "Creating ComediPulseTrainGenerator" << endlog();
        // Check if subd is counter...
        if ( _myCard->getSubDeviceType( _subDevice ) != COMEDI_SUBD_COUNTER ){
            log(Error) << "Comedi Counter : subdev is not a counter, Type = " 
                       << _myCard->getSubDeviceType(_subDevice) << endlog();
        }
        // Check how many counters this subdevice actually has
        unsigned int nchan = comedi_get_n_channels(_myCard->getDevice()->it,_subDevice);
        /* Set timebase to maximum, ie. 20 MHz for the 6601 and 80 MHz
           for the 6602 board
           FIXME:  Make this more flexible if comedi driver supports
           this.
        */
        switch(nchan){
        case 4:
	  // Max Timebase of 6601 is 20 Mhz
	  _smallest_step = PSECS_IN_SECS / 20000000;
	  break;
        case 8:
	  // Max Timebase of 6601 is 80 Mhz
	  _smallest_step = PSECS_IN_SECS / 80000000;
	  break;
        }
        if ( nchan <= _channel ){
            log(Error) << "Comedi Counter : Only " << nchan << " channels on this counter subdevice" << endlog();
        }
        /* Configure the counter subdevice
           Configure the GPCT for use as an PTG 
        */
#define PTG_CONFIG_DATA 3

        comedi_insn insn;
        Data config_data[PTG_CONFIG_DATA]; // Configuration data

        insn.insn=INSN_CONFIG;
        insn.n=0; // Irrelevant for config
        config_data[0] = INSN_CONFIG_GPCT_PULSE_TRAIN_GENERATOR;
        config_data[1] = 0; // pulse_width
        config_data[2] = 0; // pulse_period

        insn.data=config_data;
        insn.subdev=_subDevice;
        insn.chanspec=CR_PACK(_channel,0,0);

        int ret=comedi_do_insn(_myCard->getDevice()->it,&insn);
        if (ret<0)
            log(Error) << "Comedi Counter : Instruction to configure counter -> Pulse Train Generator failed, ret = " << ret << endlog();
        else
            log(Info) << "Comedi Counter now configured as Pulse Train Generator" << endlog();
    }
  
    ComediPulseTrainGenerator::~ComediPulseTrainGenerator()
    {
        comedi_insn insn;
        Data inttrig_data[1];
        int ret;
        
        insn.n=1; // MUST BE ONE (see comedi_fops.c), don't know why
        inttrig_data[0] = _channel;
        insn.data=inttrig_data;
        insn.subdev=_subDevice;
        insn.chanspec=CR_PACK(_channel,0,0);
        ret=comedi_do_insn(_myCard->getDevice()->it,&insn);
        if(ret<0){
            log(Error) << "Triggering reset on ComediPTG () failed, ret = " << ret << endlog();
        }
    }


    bool ComediPulseTrainGenerator::pulseWidthSet(psecs picos)
    {
        if (picos != _pulse_width){
            comedi_insn insn;
            Data write_data[2]; // pulse width and period

            insn.insn=INSN_WRITE;
            insn.n=0;
            write_data[0] = INSN_CONFIG_GPCT_PULSE_TRAIN_GENERATOR;
            write_data[1] = psecs_to_timebase(picos); // pulse_width
            write_data[2] = psecs_to_timebase(_pulse_period); // pulse_period

            insn.data=write_data;
            insn.subdev=_subDevice;
            insn.chanspec=CR_PACK(_channel,0,0);

            int ret=comedi_do_insn(_myCard->getDevice()->it,&insn);
            if (ret<0){
                log(Error) << "Comedi Counter : Changing the PTG period failed, ret = " << ret << endlog();
                return false;
            }
            else {
                _pulse_width = picos;
                return true;
            }
        }
        return true;
    }

    bool ComediPulseTrainGenerator::pulsePeriodSet(psecs picos)
    {
        if (picos != _pulse_period){
            comedi_insn insn;
            Data write_data[2]; // pulse width and period

            insn.insn=INSN_WRITE;
            insn.n=0;
            write_data[0] = INSN_CONFIG_GPCT_PULSE_TRAIN_GENERATOR;
            write_data[1] = psecs_to_timebase(_pulse_width); // pulse_width
            write_data[2] = psecs_to_timebase(picos); // pulse_period

            insn.data=write_data;
            insn.subdev=_subDevice;
            insn.chanspec=CR_PACK(_channel,0,0);

            int ret=comedi_do_insn(_myCard->getDevice()->it,&insn);
            if (ret<0){
                log(Error) << "Comedi Counter : Changing the PTG period failed, ret = " << ret << endlog();
                return false;
            }
            else {
                _pulse_period = picos;
                return true;
            }
        }
        return true;
    }

    bool ComediPulseTrainGenerator::start()
    {
        if (_running == false){
            comedi_insn insn;
            Data config_data[1];
            int ret;
        
            insn.insn=INSN_CONFIG;
            insn.n=0;

            config_data[0] = INSN_CONFIG_GPCT_ARM;
            insn.data=config_data;
            insn.subdev=_subDevice;
            insn.chanspec=CR_PACK(_channel,0,0);
            ret = comedi_do_insn(_myCard->getDevice()->it,&insn);
            if(ret<0){
                log(Error) << "ComediPTG::start() failed, ret = " << ret << endlog();
                return false;
            }
            _running = true;
        }
        return true;
    }
    
    bool ComediPulseTrainGenerator::stop()
    {
        if (_running == true){
            comedi_insn insn;
            Data config_data[1];
            int ret;
        
            insn.insn=INSN_CONFIG;
            insn.n=0;

            config_data[0] = INSN_CONFIG_GPCT_DISARM;
            insn.data=config_data;
            insn.subdev=_subDevice;
            insn.chanspec=CR_PACK(_channel,0,0);
            ret = comedi_do_insn(_myCard->getDevice()->it,&insn);
            if(ret<0){
                log(Error) << "ComediPTG::start() failed, ret = " << ret << endlog();
                return false;
            }
            _running = false;
        }
        return true;
    }

}
