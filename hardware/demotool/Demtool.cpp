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

#include "Demotool.hpp"


namespace Orocos
{
  using namespace RTT;
  using namespace std;
  using namespace KDL;
  
  Demotool::Demotool(string name, string propertyfile):
    GenericTaskContext(name)
    _pos_leds_demotool("pos_leds_demotool","XYZ positions of all LED markers, relative to demtool frame"),
    _mass_demotool("mass_demotool","mass of objects attached to force censor of demotool"),
    _center_gravity_demotool("center_gravity_demotool","center of gravity of mass attached to demotool"),
    _demotool_obj("demotool_obj","frame from demotool to object"),
    _demotool_fs("demotool_fs","frame from demotool to force sensor"),
    _propertyfile(propertyfile)
  {
    Logger::log()<<Logger::Debug<<this->getName()<<": adding Properties"<<Logger::endl;
    properties()->addProperty(&_pos_leds_demotool);
    properties()->addProperty(&_mass_demotool);
    properties()->addProperty(&_center_gravity_demotool);
    properties()->addProperty(&_demotool_obj);
    properties()->addProperty(&_demotool_fs);

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
  
  Demotool::~Demotool()
  {
#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
    delete _comediDev_NI6024;
    delete _comediSubdevAIn;
    for(unsigned int i=0; i<_LaserInput.size();i++)
      delete _LaserInput[i];
#endif
  }
  
  bool Demotool::startup()    
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
    
  void Demotool::update()
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
  
  void Demotool::shutdown()
  {
    writeProperties(_propertyfile);
  }
}//namespace

