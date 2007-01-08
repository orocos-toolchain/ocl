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

#include "LaserScanner.hpp"
#include <iostream>

namespace OCL
{
  using namespace RTT;
  using namespace std;
  
  LaserScanner::LaserScanner(string name, string propertyfile):
    TaskContext(name),
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
    Logger::log()<<Logger::Debug<<this->getName()<<": adding Properties"<<Logger::endl;
    properties()->addProperty(&_simulation_values);
    properties()->addProperty(&_volt2m);
    properties()->addProperty(&_offsets);
    properties()->addProperty(&_upperLimits);
    properties()->addProperty(&_lowerLimits);

    Logger::log()<<Logger::Debug<<this->getName()<<": adding Ports"<<Logger::endl;
    ports()->addPort(&_distances);

    Logger::log()<<Logger::Debug<<this->getName()<<": adding Events"<<Logger::endl;
    events()->addEvent(&_distanceOutOfRange, "Distance out of Range", "C", "Channel", "V", "Value");
    
    if(!marshalling()->readProperties(_propertyfile))
      log(Error)<<"Reading properties failed."<<endlog();
    
    Logger::log()<<Logger::Debug<<this->getName()<<": constructed."<<Logger::endl;


  }

  
  LaserScanner::~LaserScanner()
  {
  }
  

  bool LaserScanner::startup()    
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
    
    void LaserScanner::update()
    {
        _distances_local = _simulation_values.value();
        _distances.Set(_distances_local);
    }
    
    void LaserScanner::shutdown()
    {
        marshalling()->writeProperties(_propertyfile);
    }
}//namespace

