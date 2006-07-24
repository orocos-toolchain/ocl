// $Id: nAxisGeneratorCartesianPos.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
// Copyright (C) 2003 Klaas Gadeyne <klaas.gadeyne@mech.kuleuven.ac.be>
//                    Wim Meeussen  <wim.meeussen@mech.kuleuven.ac.be>
// Copyright (C) 2006 Ruben Smits <ruben.smits@mech.kuleuven.be>
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

#include "nAxesSensor.hpp"
#include <rtt/Logger.hpp>
#include <assert.h>

namespace Orocos
{
  using namespace RTT;
  using namespace std;
  
  nAxesSensor::nAxesSensor(string name,unsigned int num_axes)
    : GenericTaskContext(name),
      _num_axes(num_axes), 
      _position_local(num_axes),
      _velocity_local(num_axes),
      _position_sensors(num_axes),
      _velocity_sensors(num_axes),
      _position_naxes("nAxesSensorPosition"),
      _velocity_naxes("nAxesSensorVelocity")
  {
    
    for(int i=0;i<_num_axes;i++){
      char buf[80];
      sprintf(buf,"positionValue%d",i);
      _position_sensors[i] = new ReadDataPort<double>(buf);
      ports()->addPort(_position_sensors[i]);
      sprintf(buf,"driveValue%d",i);
      _velocity_sensors[i] = new ReadDataPort<double>(buf);
      ports()->addPort(_velocity_sensors[i]);
    }
    
    ports()->addPort(&_position_naxes);
    ports()->addPort(&_velocity_naxes);
  }
  
  nAxesSensor::~nAxesSensor(){};
  
  bool nAxesSensor::startup()
  {
    //initialize values
    for (unsigned int i=0; i<_num_axes; i++){
      _position_local[i] = _position_sensors[i]->Get();
      _velocity_local[i] = _velocity_sensors[i]->Get();
    }
    _position_naxes.Set(_position_local);
    _velocity_naxes.Set(_velocity_local);
    
    return true;
  }
  
  
  void nAxesSensor::update()
  {
    // copy values from position sensors to local variable
    for (unsigned int i=0; i<_num_axes; i++){
      _position_local[i] = _position_sensors[i]->Get();
      _velocity_local[i] = _velocity_sensors[i]->Get();
    }
        
    _position_naxes.Set(_position_local);
    _velocity_naxes.Set(_velocity_local);
  }
  
  void nAxesSensor::shutdown()
  {
  }
  
}//namespace


