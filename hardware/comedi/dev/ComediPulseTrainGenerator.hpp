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

namespace RTT
{
  /**
   * A class for generation of pulse trains using comedi the hardware
   * abstraction layer.  Based on the comedi API of the home
   * written driver for the NI660X card. 
   * @warning It is very unlikely that this class will work for your
   * hardware as it requires a patched comedi device driver !
   * See <http://people.mech.kuleuven.ac.be/~kgadeyne/linux/> for more
   * information about all this stuff
   * @todo subdevice locking
   * @todo be more flexible in chosing the timebase, but this is
   * currently not necessary as the comedi driver does not support
   * this yet
   */
  class ComediPulseTrainGenerator :
    public PulseTrainGeneratorInterface
  {
  public:
    /**
     * Create a nameserved pulse train generator.
     *
     * @param cd The comedi device your are using
     * @param subd The comedi subdevice where the COUNTER is situated.
     * @param channel The number of the pulse train generator on the comedi subdevice
     * @param name  The name of the pulse train generator.
     */
    ComediPulseTrainGenerator(ComediDevice * cd, unsigned int subd, 
			      unsigned int channel, const std::string& name);

    /**
     * Create an pulse train generator.
     *
     * @param cd The comedi device your are using
     * @param subd The comedi subdevice where the COUNTER is situated.
     * @param channel The number of the pulse train generator on the comedi subdevice
     */
    ComediPulseTrainGenerator(ComediDevice * cd, unsigned int subd, unsigned int channel);

    virtual ~ComediPulseTrainGenerator();

    // Redefinition of Pure virtuals    
    virtual bool pulseWidthSet(psecs picos);
    virtual bool pulsePeriodSet(psecs picos);
    virtual bool start();
    virtual bool stop();

  protected:
    // All constructors use this method
    void init();
    /// Convert picoseconds to a multiple of the chosen Timebase
    unsigned int psecs_to_timebase(psecs picos);
    
    ComediDevice * _myCard;
    unsigned int _subDevice;
    unsigned int _channel;
    
    psecs _pulse_width;
    psecs _pulse_period;
    /// Smallest step (related to the chosen timebase on the board)
    /// This value is not certain to be an integer, even in pico seconds.
    double _smallest_step;
    bool _running;
  };

};

#endif

