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
#include "comedi_common.h"

namespace OCL
{
    typedef unsigned int Data;

    ComediPulseTrainGenerator::ComediPulseTrainGenerator(ComediDevice * cd, unsigned int subd, 
                                                         unsigned int encNr, const std::string& name)
        : PulseTrainGeneratorInterface(name),
          _myCard(cd), _subDevice(subd), _channel(encNr),
          _pulse_width(1000000), _pulse_period(1000000), _running(false)
    {
        init();
    }

    ComediPulseTrainGenerator::ComediPulseTrainGenerator(ComediDevice * cd, unsigned int subd, 
                                                         unsigned int encNr)
      :  _myCard(cd), _subDevice(subd), _channel(encNr),
	 _pulse_width(1000000), _pulse_period(1000000), _running(false)
    {
        init();
    }

    unsigned int ComediPulseTrainGenerator::psecs_to_timebase(psecs picos)
    {
        // Disable Logger statements since these they can be
        // called from a RT thread
        // Logger::In in("ComediPulseTrainGenerator");
        if (picos < _smallest_step){
            /*
            log(Warning) << "ComediPTG: Period or Pulsewidth too small for current timebase, picos = " << picos 
		       << ".  Returning ZERO" << endlog();
            */
            return 0;
        }
        /*
        if (picos % _smallest_step != 0){
            log(Warning) << "ComediPTG: " << picos << " is not a multiple of " << _smallest_step 
                         << " -> There will be a rounding error" << endlog();
        }
        */
        // rounding:
        return (unsigned int)round( double(picos) / _smallest_step );
    }
    
    void ComediPulseTrainGenerator::init()
    {
        Logger::In in("ComediPulseTrainGenerator");
        log(Info) << "Creating ComediPulseTrainGenerator" << endlog();
        // Check if subd is counter...
        if ( _myCard->getSubDeviceType( _subDevice ) != COMEDI_SUBD_COUNTER ){
            log(Error) << "Comedi Counter : subdev is not a counter, Type = " 
                       << _myCard->getSubDeviceType(_subDevice) << endlog();
            // Toggle error condition:
            _myCard = 0;
            return;
        }
        // Check how many counters this subdevice actually has
        unsigned int nchan = comedi_get_n_channels(_myCard->getDevice()->it,_subDevice);
        /* Set timebase to maximum, ie. 20 MHz for the 6601 and 80 MHz
           for the 6602 board
           FIXME:  Make this more flexible if comedi driver supports
           this.
        */
        const unsigned clock_period_ns;
        const unsigned tbase;
        comedi_t *device = _myCard->getDevice()->it;
        switch(nchan){
        case 4:
            // Max Timebase of 6601 is 20 Mhz
            _smallest_step = PSECS_IN_SECS / 20000000;
            clock_period_ns = 50;  /* 20MHz clock */
            tbase = NI_GPCT_TIMEBASE_1_CLOCK_SRC_BITS;
            break;
        case 8:
            // Max Timebase of 6602 is 80 Mhz
            _smallest_step = PSECS_IN_SECS / 80000000;
            clock_period_ns = 12;  /* 80MHz clock */
            tbase = NI_GPCT_TIMEBASE_3_CLOCK_SRC_BITS;
            break;
        }
        if ( nchan <= _channel ){
            log(Error) << "Comedi Counter : Only " << nchan << " channels on this counter subdevice" << endlog();
        }
        /* Configure the counter subdevice
           Configure the GPCT for use as an PTG 
        */
        counter_mode = NI_GPCT_COUNTING_MODE_NORMAL_BITS;
        // toggle output on terminal count
        counter_mode |= NI_GPCT_OUTPUT_TC_TOGGLE_BITS;
        // load on terminal count
        counter_mode |= NI_GPCT_LOADING_ON_TC_BIT;
        // alternate the reload source between the load a and load b registers
        counter_mode |= NI_GPCT_RELOAD_SOURCE_SWITCHING_BITS;
        // count down
        counter_mode |= NI_GPCT_COUNTING_DIRECTION_DOWN_BITS;
        // initialize load source as load b register
        counter_mode |= NI_GPCT_LOAD_B_SELECT_BIT;
        // don't stop on terminal count
        counter_mode |= NI_GPCT_STOP_ON_GATE_BITS;
        // don't disarm on terminal count or gate signal
        counter_mode |= NI_GPCT_NO_HARDWARE_DISARM_BITS;
        retval = set_counter_mode(device, _subDevice, counter_mode);
        if(retval < 0) return retval;

        /* 20MHz clock */
        retval = set_clock_source(device, _subDevice, tbase, clock_period_ns, channel);
        if(retval < 0) return retval;

        up_ticks = (up_time_ns + clock_period_ns / 2) / clock_period_ns;
        down_ticks = (period_ns + clock_period_ns / 2) / clock_period_ns - up_ticks;
        /* set initial counter value by writing to channel 0 */
        retval = comedi_data_write(device, _subDevice, 0, 0, 0, down_ticks);
        if(retval < 0) return retval;
        /* set "load a" register to the number of clock ticks the counter output should remain low
           by writing to channel 1. */
        comedi_data_write(device, _subDevice, 1, 0, 0, down_ticks);
        if(retval < 0) return retval;
        /* set "load b" register to the number of clock ticks the counter output should remain high
           by writing to channel 2 */
        comedi_data_write(device, _subDevice, 2, 0, 0, up_ticks);
        if(retval < 0) return retval;

        /*FIXME: check that device is counter */
        printf("Generating pulse train on _subDevice %d.\n", _subDevice);
        printf("Period = %d ns.\n", period_ns);
        printf("Up Time = %d ns.\n", up_time);
        printf("Down Time = %d ns.\n", period_ns - up_time);

        if (ret<0)
            log(Error) << "Comedi Counter : Instruction to configure counter -> Pulse Train Generator failed, ret = " << ret << endlog();
        else
            log(Info) << "Comedi Counter now configured as Pulse Train Generator" << endlog();
    }
  
    ComediPulseTrainGenerator::~ComediPulseTrainGenerator()
    {
        Logger::In in("ComediPulseTrainGenerator");
        if (!_myCard)
            return;

        if(ret<0){
            log(Error) << "Triggering reset on ComediPTG () failed, ret = " << ret << endlog();
        }
    }


    bool ComediPulseTrainGenerator::pulseWidthSet(psecs picos)
    {
        Logger::In in("ComediPulseTrainGenerator");
        if (!_myCard)
            return false;

	if (picos != _pulse_width){
	  if (_running == true){ // If counter is ARMED, use INSN_WRITE
            comedi_insn insn;
            Data write_data[2]; // pulse width and period
            insn.insn=INSN_WRITE;
            insn.n=1;
            write_data[0] = this->psecs_to_timebase(picos); // pulse_width
            write_data[1] = this->psecs_to_timebase(_pulse_period); // pulse_period
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
	  else { // use INSN_CONFIG if ctr is unarmed
	    comedi_insn insn;
	    Data config_data[PTG_CONFIG_DATA]; // Configuration data
	    insn.insn=INSN_CONFIG;
	    insn.n=1; /* Should be irrelevant for config, but is not for gnulinux! */
	    config_data[0] = INSN_CONFIG_GPCT_PULSE_TRAIN_GENERATOR;
	    config_data[1] = psecs_to_timebase(picos); // pulse_width
	    config_data[2] = psecs_to_timebase(_pulse_period); // pulse_period
	    insn.data=config_data;
	    insn.subdev=_subDevice;
	    insn.chanspec=CR_PACK(_channel,0,0);
	    int ret=comedi_do_insn(_myCard->getDevice()->it,&insn);
	    if (ret<0){
	      log(Error) << "Comedi Counter : Instruction to configure counter -> Pulse Train Generator failed, ret = " << ret << endlog();
	      return false;
	    }
	    else {
	      _pulse_width = picos;
	      return true;
	    }
	  }
	}
	return true;
    }

    bool ComediPulseTrainGenerator::pulsePeriodSet(psecs picos)
    {
        Logger::In in("ComediPulseTrainGenerator");
        if (!_myCard)
            return false;

        if (picos != _pulse_period){ // Only handle if necessary
	  if (_running == true){ // If counter is ARMED, use INSN_WRITE
	    comedi_insn insn;
            Data write_data[2]; // pulse width and period
            insn.insn=INSN_WRITE;
            insn.n=1;
            write_data[0] = psecs_to_timebase(_pulse_width); // pulse_width
            write_data[1] = psecs_to_timebase(picos); // pulse_period
            // log(Info) << "PTG 0 = " << write_data[0] << " 1 = " << write_data[1] << endlog();
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
	  else { // Counter unarmed, use insn_config
	    comedi_insn insn;
	    Data config_data[PTG_CONFIG_DATA]; // Configuration data
	    insn.insn=INSN_CONFIG;
	    insn.n=1; /* Should be irrelevant for config, but is not for gnulinux! */
	    config_data[0] = INSN_CONFIG_GPCT_PULSE_TRAIN_GENERATOR;
	    config_data[1] = psecs_to_timebase(_pulse_width); // pulse_width
	    config_data[2] = psecs_to_timebase(picos); // pulse_period
	    insn.data=config_data;
	    insn.subdev=_subDevice;
	    insn.chanspec=CR_PACK(_channel,0,0);
	    int ret=comedi_do_insn(_myCard->getDevice()->it,&insn);
	    if (ret<0){
	      log(Error) << "Comedi Counter : Instruction to configure counter -> Pulse Train Generator failed, ret = " << ret << endlog();
	      return false;
	    }
	    else {
	      _pulse_period = picos;
	      return true;
	    }
	  }
	}
	return true; // period already ok
    }
  
    bool ComediPulseTrainGenerator::start()
    {
        Logger::In in("ComediPulseTrainGenerator");
        if (!_myCard)
            return false;
        if (_running == false){

            int ret = arm(_myCard->getDevice()->it, _subDevice, NI_GPCT_ARM_IMMEDIATE, _channel);
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
        Logger::In in("ComediPulseTrainGenerator");
        if (!_myCard)
            return false;
        if (_running == true){
            int ret = reset_counter(_myCard->getDevice()->it, _subDevice, _channel);
            if(ret<0){
                log(Error) << "ComediPTG::stop() failed, ret = " << ret << endlog();
            }
            _running = false;
        }
        return true;
    }

}
