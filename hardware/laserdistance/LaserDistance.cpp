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

#include "LaserDistance.hpp"
#include <iostream>

#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
#define NR_CHAN 2
#define OFFSET 0
#endif

namespace OCL
{
  using namespace RTT;
  using namespace std;

  LaserDistance::LaserDistance(string name,unsigned int nr_chan ,string propertyfile):
    TaskContext(name),
#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
    _LaserInput(nr_chan),
#endif
    _nr_chan(nr_chan),
    _simulation_values("sim_values","Value used for simulation",vector<double>(nr_chan,0)),
    _volt2m("volt2m","Convert Factor from volt to m",vector<double>(nr_chan,0)),
    _offsets("offsets","Offset in m",vector<double>(nr_chan,0)),
    _lowerLimits("low_limits","LowerLimits of the distance sensor",vector<double>(nr_chan,0)),
    _upperLimits("up_limits","UpperLimits of the distance sensor",vector<double>(nr_chan,0)),
    _distances("LaserDistance",vector<double>(nr_chan,0)),
    _distanceOutOfRange("distanceOutOfRange"),
    _measurement(nr_chan),
    _distances_local(nr_chan),
    _propertyfile(propertyfile)
  {
    log(Debug) <<this->getName()<<": adding Properties"<<endlog();
    properties()->addProperty(&_simulation_values);
    properties()->addProperty(&_volt2m);
    properties()->addProperty(&_offsets);
    properties()->addProperty(&_upperLimits);
    properties()->addProperty(&_lowerLimits);

    log(Debug) <<this->getName()<<": adding Ports"<<endlog();
    ports()->addPort(&_distances);

    log(Debug) <<this->getName()<<": adding Events"<<endlog();
    events()->addEvent(&_distanceOutOfRange, "Distance out of Range", "C", "Channel", "V", "Value");

#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
    if(_nr_chan>NR_CHAN){
      log(Warning) <<"Only 2 hardware sensors currently available, resetting nr of channels to 2"<<endlog();
      _nr_chan = NR_CHAN;
    }
    log(Debug) <<this->getName()<<": Creating ComediDevice"<<endlog();
    _comediDev_NI6024  = new ComediDevice( 4 ); //NI-6024 for analog in
    int subd;
    subd = 0; // subdevice 0 is analog  in
    log(Debug) <<this->getName()<<": Creating ComediSubDevice"<<endlog();
    _comediSubdevAIn     = new ComediSubDeviceAIn( _comediDev_NI6024, "Laser", subd );
    for(unsigned int i = 0; i < nr_chan;i++){
      log(Debug) <<this->getName()<<": Creating AnalogInput "<<i<<endlog();
      _LaserInput[i] = new AnalogInput(_comediSubdevAIn, i+OFFSET); //channel number starting from 0
    }
#endif

    if(!marshalling()->readProperties(_propertyfile))
      log(Error)<<"Reading properties failed."<<endlog();

    log(Debug) <<this->getName()<<": constructed."<<endlog();


  }

  LaserDistance::~LaserDistance()
  {
#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
    delete _comediDev_NI6024;
    delete _comediSubdevAIn;
    for(unsigned int i=0; i<_LaserInput.size();i++)
      delete _LaserInput[i];
#endif
  }

  bool LaserDistance::startup()
  {
    if(_simulation_values.value().size()!=_nr_chan||
       _volt2m.value().size()!=_nr_chan||
       _offsets.value().size()!=_nr_chan||
       _upperLimits.value().size()!=_nr_chan||
       _lowerLimits.value().size()!=_nr_chan)
	{
	  log(Error) <<"size of Properties do not match nr of channels"<<endlog();
	  return false;
	}
    return true;
  }

    void LaserDistance::update()
    {
#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
        for(unsigned int i = 0 ; i<_nr_chan;i++){
            _measurement[i] = _LaserInput[i]->value();
            _distances_local[i] = _volt2m.value()[i]*_measurement[i]+_offsets.value()[i];
            if(_distances_local[i]<_lowerLimits.value()[i]||
               _distances_local[i]>_upperLimits.value()[i])
                _distanceOutOfRange(i,_distances_local[i]);
        }
#else
        _distances_local = _simulation_values.value();
#endif
        _distances.Set(_distances_local);
    }

    void LaserDistance::shutdown()
    {
        marshalling()->writeProperties(_propertyfile);
    }
}//namespace

