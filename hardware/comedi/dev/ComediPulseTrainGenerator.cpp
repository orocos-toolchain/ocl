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
                                                         const std::string& name)
        : PulseTrainGeneratorInterface(name),
          _myCard(cd), _subDevice(subd),
          _pulse_width(1000000), _pulse_period(1000000), _running(false)
    {
        init();
    }

    ComediPulseTrainGenerator::ComediPulseTrainGenerator(ComediDevice * cd, unsigned int subd)
      :  _myCard(cd), _subDevice(subd),
	 _pulse_width(1000000), _pulse_period(1000000), _running(false)
    {
        init();
    }

    unsigned int ComediPulseTrainGenerator::psecs_to_timebase(psecs picos)
    {
        if (picos < _smallest_step){
            return 0;
        }
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
        unsigned int nchan = 0;
        /* Set timebase to maximum, ie. 20 MHz for the 6601 and 80 MHz
           for the 6602 board
           FIXME:  Make this more flexible if comedi driver supports
           this.
        */
#if OROCOS_TARGET_GNULINUX
        const char* bn = comedi_get_board_name(_myCard->getDevice()->it);
        if ( strncmp(bn, "PCI-6602",8) == 0 )
            nchan = 8;
        if ( strncmp(bn, "PCI-6601",8) == 0 )
            nchan = 4;
#else
        // not get_board_name not supported in LXRT.
        int sdevs = comedi_get_n_subdevices(_myCard->getDevice()->it);
        if ( sdevs == 10 )
            nchan = 8;
        if ( sdevs == 6 )
            nchan = 4;
#endif
        
        unsigned tbase = 0;
        switch(nchan){
        case 8:
            // Max Timebase of 6602 is 80 Mhz
            _smallest_step = PSECS_IN_SECS / 80000000;
            _clock_period = 12500;  /* 80MHz clock */
            tbase = NI_GPCT_TIMEBASE_3_CLOCK_SRC_BITS;
            log(Info) << "Detected NI-6602."<<endlog();
            break;
        default:
            log(Warning) << "Unknown Comedi Counter card? Could not determine timebase. Using a default."<<endlog();
        case 4:
            // Max Timebase of 6601 is 20 Mhz
            _smallest_step = PSECS_IN_SECS / 20000000;
            _clock_period = 50000;  /* 20MHz clock */
            tbase = NI_GPCT_TIMEBASE_1_CLOCK_SRC_BITS;
            log(Info) << "Detected NI-6601."<<endlog();
            break;
        }

        /* Configure the counter subdevice
           Configure the GPCT for use as an PTG 
        */
        unsigned counter_mode = NI_GPCT_COUNTING_MODE_NORMAL_BITS;
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
        int retval = set_counter_mode(_myCard->getDevice()->it, _subDevice, counter_mode);
        if(retval < 0) return;

        /* clock period may be rounded, tbase matters. */
        retval = set_clock_source(_myCard->getDevice()->it, _subDevice, tbase, _clock_period/1000);
        if(retval < 0) return;

        if (retval<0)
            log(Error) << "Comedi Counter : Instruction to configure counter -> Pulse Train Generator failed, ret = " << retval << endlog();
        else
	  log(Info) << "Comedi Counter "<< _subDevice << " now configured as Pulse Train Generator" << endlog();
    }
  
    ComediPulseTrainGenerator::~ComediPulseTrainGenerator()
    {
        Logger::In in("ComediPulseTrainGenerator");
        if (!_myCard)
            return;

        int ret = reset_counter(_myCard->getDevice()->it, _subDevice);
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

            unsigned up_ticks = ( picos + _clock_period / 2) / _clock_period;
            unsigned down_ticks = ( _pulse_period + _clock_period / 2) / _clock_period - up_ticks;
            /* set initial counter value by writing to channel 0 */
            int retval = comedi_data_write(_myCard->getDevice()->it, _subDevice, 0, 0, 0, down_ticks);
            if(retval < 0) return false;
            /* set "load a" register to the number of clock ticks the counter output should remain low
               by writing to channel 1. */
            retval = comedi_data_write(_myCard->getDevice()->it, _subDevice, 1, 0, 0, down_ticks);
            if(retval < 0) return false;
            /* set "load b" register to the number of clock ticks the counter output should remain high
               by writing to channel 2 */
            retval = comedi_data_write(_myCard->getDevice()->it, _subDevice, 2, 0, 0, up_ticks);
            if(retval < 0) return false;

            _pulse_width = picos;
#if 0
            printf("Generating pulse train on sub device %d.\n", _subDevice);
            printf("Period = %lld ps.\n", _pulse_period);
            printf("Up Time = %lld ps.\n", _pulse_width);
            printf("Down Time = %lld ps.\n", _pulse_period - _pulse_width);
#endif
        }
        return true;
    }

    bool ComediPulseTrainGenerator::pulsePeriodSet(psecs picos)
    {
        Logger::In in("ComediPulseTrainGenerator");
        if (!_myCard)
            return false;

        if (picos != _pulse_period){ // Only handle if necessary

            unsigned up_ticks = (_pulse_width + _clock_period / 2) / _clock_period;
            unsigned down_ticks = (picos + _clock_period / 2) / _clock_period - up_ticks;
            /* set initial counter value by writing to channel 0 */
            int retval = comedi_data_write(_myCard->getDevice()->it, _subDevice, 0, 0, 0, down_ticks);
            if(retval < 0) return false;
            /* set "load a" register to the number of clock ticks the counter output should remain low
               by writing to channel 1. */
            retval = comedi_data_write(_myCard->getDevice()->it, _subDevice, 1, 0, 0, down_ticks);
            if(retval < 0) return false;
            /* set "load b" register to the number of clock ticks the counter output should remain high
               by writing to channel 2 */
            retval = comedi_data_write(_myCard->getDevice()->it, _subDevice, 2, 0, 0, up_ticks);
            if(retval < 0) return false;

            _pulse_period = picos;
#if 0
            printf("Generating pulse train on sub device %d.\n", _subDevice);
            printf("Period = %lld ps.\n", _pulse_period);
            printf("Up Time = %lld ps.\n", _pulse_width);
            printf("Down Time = %lld ps.\n", _pulse_period - _pulse_width);
#endif
        }
        return true; // period already ok
    }
  
    bool ComediPulseTrainGenerator::start()
    {
        Logger::In in("ComediPulseTrainGenerator");
        if (!_myCard)
            return false;
        if (_running == false){

            int ret = arm(_myCard->getDevice()->it, _subDevice, NI_GPCT_ARM_IMMEDIATE);
            if(ret<0){
                log(Error) << "ComediPTG "<<_subDevice<<": start() failed, ret = " << ret << endlog();
                log(Error) << "pulse width: " << _pulse_width << ", pulse period: " << _pulse_period <<endlog();
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
            int ret = reset_counter(_myCard->getDevice()->it, _subDevice);
            if(ret<0){
                log(Error) << "ComediPTG::stop() failed, ret = " << ret << endlog();
                return false;
            }
            _running = false;
        }
        return true;
    }

}
