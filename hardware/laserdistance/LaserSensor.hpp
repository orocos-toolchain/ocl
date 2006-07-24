// Copyright (C) 2006 Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
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

#ifndef __LASER_HARDWARE__
#define __LASER_HARDWARE__

#include <pkgconf/system.h>

#include <rtt/RTT.hpp>
#include <rtt/GenericTaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Properties.hpp>

#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
#include <rtt/dev/ComediDevice.hpp>
#include <rtt/dev/ComediSubDeviceAIn.hpp>
#include <rtt/dev/AnalogInput.hpp>
#endif

namespace Orocos
{
    
  class LaserSensor : public RTT::GenericTaskContext
  {
  public:
    /**
     * Construct an interface to the automated Laser
     * initialising a link to the digital IO PCI cards
     */
    LaserSensor(std::string name,unsigned int nr_chan,std::string propertyfilename="cpf/LaserSensor.cpf");
    virtual~LaserSensor();
    
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
    
    //double getDistance(int channel);
        
  private:
#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
    RTT::ComediDevice* _comediDev_NI6024; //NI-6024 for analog in
    RTT::ComediSubDeviceAIn* _comediSubdevAIn;
    std::vector<RTT::AnalogInput<unsigned int>*> _LaserInput;
#endif
    
    unsigned int _nr_chan;
    
    RTT::Property<std::vector<double> > _simulation_values;
    RTT::Property<std::vector<double> > _volt2m;
    RTT::Property<std::vector<double> > _offsets;
    RTT::Property<std::vector<double> > _lowerLimits;
    RTT::Property<std::vector<double> > _upperLimits;
    
    RTT::WriteDataPort<std::vector<double> > _distances;
    
    RTT::Event< void(int,double)>  _distanceOutOfRange;
    RTT::EventC _outOfRangeEvent;
    
    std::vector<double> _measurement, _distances_local;
    std::string _propertyfile;
    
    
   }; // class
} // namespace

#endif
