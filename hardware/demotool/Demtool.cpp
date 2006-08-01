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

#include "LaserSensor.hpp"

#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
#define NR_CHAN 2
#define OFFSET 0
#endif

namespace Orocos
{
  using namespace RTT;
  using namespace std;
  
  LaserSensor::LaserSensor(string name,unsigned int nr_chan ,string propertyfile):
    GenericTaskContext(name),
    _nr_chan(nr_chan),
#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)    
    _LaserInput(nr_chan),
#endif
    _simulation_values("sim_values","Value used for simulation"),
    _volt2m("volt2m","Convert Factor from volt to m"),
    _offsets("offsets","Offset in m"),
    _lowerLimits("low_limits","LowerLimits of the distance sensor"),
    _upperLimits("up_limits","UpperLimits of the distance sensor"),
    _distances("LaserDistance"),
    _measurement(nr_chan),
    _distances_local(nr_chan),
    _propertyfile(propertyfile)
  {
    Logger::log()<<Logger::Debug<<this->getName()<<": adding Properties"<<Logger::endl;
    properties()->addProperty(&_simulation_values);
    properties()->addProperty(&_volt2m);
    properties()->addProperty(&_offsets);
    properties()->addProperty(&_upperLimits);
    properties()->addProperty(&_lowerLimits);

    Logger::log()<<Logger::Debug<<this->getName()<<": adding Ports"<<Logger::endl;
    ports()->addPort(&_distances);

    Logger::log()<<Logger::Debug<<this->getName()<<": adding Events"<<Logger::endl;
    events()->addEvent("distanceOutOfRange",&_distanceOutOfRange);
    
#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
    if(_nr_chan>NR_CHAN){
      Logger::log()<<Logger::Warning<<"Only 2 hardware sensors currently available, resetting nr of channels to 2"<<Logger::endl;
      _nr_chan = NR_CHAN;
    }
    Logger::log()<<Logger::Debug<<this->getName()<<": Creating ComediDevice"<<Logger::endl;
    _comediDev_NI6024  = new ComediDevice( 4 ); //NI-6024 for analog in
    int subd;
    subd = 0; // subdevice 0 is analog  in
    Logger::log()<<Logger::Debug<<this->getName()<<": Creating ComediSubDevice"<<Logger::endl;
    _comediSubdevAIn     = new ComediSubDeviceAIn( _comediDev_NI6024, "Laser", subd );
    for(unsigned int i = 0; i < nr_chan;i++){
      Logger::log()<<Logger::Debug<<this->getName()<<": Creating AnalogInput "<<i<<Logger::endl;
      _LaserInput[i] = new AnalogInput<unsigned int>(_comediSubdevAIn, i+OFFSET); //channel number starting from 0
    }
#endif
    
    if(!readProperties(_propertyfile))
      log(Error)<<"Reading properties failed."<<endlog();
    
    Logger::log()<<Logger::Debug<<this->getName()<<": constructed."<<Logger::endl;


  }
  
  LaserSensor::~LaserSensor()
  {
#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
    delete _comediDev_NI6024;
    delete _comediSubdevAIn;
    for(unsigned int i=0; i<_LaserInput.size();i++)
      delete _LaserInput[i];
#endif
  }
  
  bool LaserSensor::startup()    
  {
    if(_simulation_values.value().size()!=_nr_chan||
       _volt2m.value().size()!=_nr_chan||
       _offsets.value().size()!=_nr_chan||
       _upperLimits.value().size()!=_nr_chan||
       _lowerLimits.value().size()!=_nr_chan)
	{
	  Logger::log()<<Logger::Error<<"size of Properties do not match nr of channels"<<Logger::endl;
	  return false;
	}
    return true;
  }
    
  void LaserSensor::update()
  {
#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
    for(unsigned int i = 0 ; i<_nr_chan;i++){
      _measurement[i] = _LaserInput[i]->value();
      _distances_local[i] = _volt2m.value()[i]*_measurement[i]+_offsets.value()[i];
    }
#else
    _distances_local = _simulation_values.value();
#endif
    _distances.Set(_distances_local);
  }
  
  void LaserSensor::shutdown()
  {
    writeProperties(_propertyfile);
  }
}//namespace

