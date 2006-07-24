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

#include "CartesianSensor.hpp"
#include <rtt/Logger.hpp>
#include <assert.h>

namespace Orocos
{
  using namespace RTT;
  using namespace ORO_Geometry;
  using namespace std;
  
  CartesianSensor::CartesianSensor(string name,unsigned int num_axes,
					       string kine_comp_name)
    : nAxesSensor(name,num_axes),
      _frame("CartesianSensorPosition"),
      _twist("CartesianSensorVelocity"),
      _kine_comp_name(kine_comp_name)
  {
    Toolkit::Import( GeometryToolkit );
    ports()->addPort(&_frame);
    ports()->addPort(&_twist);
    
  }
  
  CartesianSensor::~CartesianSensor(){};
  
  bool CartesianSensor::startup()
  {
    try{
      _positionForward = getPeer(_kine_comp_name)->methods()->create("this","positionForward").arg(_position_local).arg(_frame_local);
      _velocityForward = getPeer(_kine_comp_name)->methods()->create("this","velocityForward").arg(_position_local).arg(_velocity_local).arg(_frame_local).arg(_twist_local);
    }
    catch(...){
      return false;
    }
    
    
    bool succes = true;

    //initialize values
    succes &= nAxesSensor::startup();
        
    succes &=_positionForward.execute();
    succes &= _velocityForward.execute();
    
    _frame.Set(_frame_local);
    _twist.Set(_twist_local);
    
    return succes;
  }
  
  
  void CartesianSensor::update()
  {
    nAxesSensor::update();
        
    _positionForward.execute();
    _velocityForward.execute();
        
    _frame.Set(_frame_local);
    _twist.Set(_twist_local);
  }
  
  void CartesianSensor::shutdown()
  {
    nAxesSensor::shutdown();
  }
  
}//namespace


