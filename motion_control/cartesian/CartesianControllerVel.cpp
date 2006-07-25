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

#include "CartesianControllerVel.hpp"
#include <assert.h>

namespace Orocos
{
  
  using namespace RTT;
  using namespace KDL;
  using namespace std;
  
  
  CartesianControllerVel::CartesianControllerVel(string name,string propertyfile)
    : GenericTaskContext(name),
      _propertyfile(propertyfile),
      _position_meas("CartesianSensorPosition"),
      _velocity_desi("CartesianDesiredVelocity"),
      _velocity_out("CartesianOutputVelocity"),
      _controller_gain("K", "Proportional Gain"),
      _is_initialized(false)
  {
    Toolkit::Import( GeometryToolkit );
    //Creating TaskContext

    //Adding Ports
    this->ports()->addPort(&_position_meas);
    this->ports()->addPort(&_velocity_desi);
    this->ports()->addPort(&_velocity_out);
    
    //Adding Properties
    this->properties()->addProperty(&_controller_gain);

    if(!readProperties(_propertyfile))
      Logger::log()<<Logger::Error<<"(CartesianControllerVel) Reading Properties from "<<_propertyfile<<" failed!!"<<Logger::endl;

  }
  
  
  CartesianControllerVel::~CartesianControllerVel(){};
  
  
  bool CartesianControllerVel::startup()
  {
    
    // check size of properties
    if(_controller_gain.value().size() != 6)
      return false;

    return true;
  }
  
  void CartesianControllerVel::update()
  {
    // copy Input and Setpoint to local values
    _position_meas_local = _position_meas.Get();
    _velocity_desi_local = _velocity_desi.Get();
    
    // initialize
    if (!_is_initialized){
      _is_initialized = true;
      _position_integrated = _position_meas_local;
      _time_begin = TimeService::Instance()->getTicks();
    }
  
    // integrate velocity
    double time_difference = TimeService::Instance()->secondsSince(_time_begin);
    _time_begin = TimeService::Instance()->getTicks();
    _position_integrated = addDelta(_position_integrated, _velocity_desi_local, time_difference);
  
    // position feedback on integrated velocity
    _velocity_feedback = diff(_position_meas_local, _position_integrated);
    for(unsigned int i=0; i<6; i++)
      _velocity_feedback(i) *= _controller_gain.value()[i];
  
    // feedback + feedforward
    _velocity_out_local = _velocity_desi_local + _velocity_feedback;
  
    _velocity_out.Set(_velocity_out_local);
  }
  
  void CartesianControllerVel::shutdown()
  {
  }
}//namespace



