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


#ifndef __COMEDI_PULSE_TRAIN_GENERATOR_HPP
#define __COMEDI_PULSE_TRAIN_GENERATOR_HPP


#include <rtt/dev/PulseTrainGeneratorInterface.hpp>
#include "ComediDevice.hpp"
#include <string>
#include <rtt/Time.hpp>

namespace OCL
{
  /**
   * A class for generation of pulse trains using the comedi hardware
   * abstraction layer. Tested with the NI6602 card. It will try to detect the
   * NI6601 or NI6602. It needs to know the card to set the correct time base.
   * Any other card is currently untested.
   * @todo subdevice locking
   */
  class ComediPulseTrainGenerator :
     public RTT::PulseTrainGeneratorInterface
  {
  public:
    /**
     * Create a nameserved pulse train generator.
     *
     * @param cd The comedi device your are using
     * @param subd The comedi subdevice where the COUNTER is situated.
     * @param name  The name of the pulse train generator.
     */
    ComediPulseTrainGenerator(ComediDevice * cd, unsigned int subd,
                              const std::string& name);

    /**
     * Create an pulse train generator.
     *
     * @param cd The comedi device your are using
     * @param subd The comedi subdevice where the COUNTER is situated.
     */
    ComediPulseTrainGenerator(ComediDevice * cd, unsigned int subd);

    virtual ~ComediPulseTrainGenerator();

    // Redefinition of Pure virtuals
    virtual bool pulseWidthSet(RTT::psecs picos);
    virtual bool pulsePeriodSet(RTT::psecs picos);
    virtual bool start();
    virtual bool stop();

  protected:
    // All constructors use this method
    void init();
    /// Convert picoseconds to a multiple of the chosen Timebase
    unsigned int psecs_to_timebase(RTT::psecs picos);

    ComediDevice * _myCard;
    unsigned int _subDevice;

    RTT::psecs _pulse_width;
    RTT::psecs _pulse_period;
    RTT::psecs _clock_period;
    /// Smallest step (related to the chosen timebase on the board)
    /// This value is not certain to be an integer, even in pico seconds.
    double _smallest_step;
    bool _running;
  };

};

#endif

