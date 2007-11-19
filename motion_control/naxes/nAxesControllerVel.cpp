// $Id: nAxisGeneratorCartesianPos.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
// Copyright (C) 2003 Klaas Gadeyne <klaas.gadeyne@mech.kuleuven.ac.be>
//                    Wim Meeussen  <wim.meeussen@mech.kuleuven.ac.be>
// Copyright (C) 2006 Ruben Smits <ruben.smits@mech.kuleuven.ac.be>
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

#include "nAxesControllerVel.hpp"

#include <assert.h>

namespace OCL{
  
    using namespace RTT;
    using namespace std;
    typedef nAxesControllerVel MyType;
    
    nAxesControllerVel::nAxesControllerVel(string name, unsigned int num_axes, 
                                           string propertyfile)
        : TaskContext(name),
          _num_axes(num_axes), 
          _propertyfile(propertyfile),
          _position_meas_local(num_axes),
          _position_desi_local(num_axes),
          _velocity_desi_local(num_axes),
          _velocity_out_local(num_axes),
          _reset_all( "resetAll", &MyType::resetAll, this),
          _resetAxis( "resetAxis", &MyType::resetAxis, this),
          _position_meas("nAxesSensorPosition"),
          _velocity_desi("nAxesDesiredVelocity"),
          _velocity_out("nAxesOutputVelocity"),
          _controller_gain("K", "Proportional Gain"),
          _is_initialized(num_axes),
          _time_begin(num_axes)
    {
        //Creating TaskContext
        
        //Adding Ports
        this->ports()->addPort(&_position_meas);
        this->ports()->addPort(&_velocity_desi);
        this->ports()->addPort(&_velocity_out);
    
        //Adding Properties
        this->properties()->addProperty(&_controller_gain);
  
        //Adding Methods
        this->methods()->addMethod( &_reset_all,"reset all axes of the controller");
        this->methods()->addMethod( &_resetAxis,"reset a single axis of the controller",
                                    "axis","axis to reset");  
        
        if(!marshalling()->readProperties(_propertyfile))
            log(Error) <<"(nAxesControllerVel) Reading Properties from "<<_propertyfile<<" failed!!"<<endlog();
        
    }
  
  
    nAxesControllerVel::~nAxesControllerVel(){};
  
    bool nAxesControllerVel::startup()
    {
  
        // check size of properties
        if(_controller_gain.value().size() != _num_axes)
            return false;
    
        // reset integrator
        for(unsigned int i=0; i<_num_axes; i++)
            _is_initialized[i] = false;
        
        return true;
    }
  
    void nAxesControllerVel::update()
    {
        // copy Input and Setpoint to local values
        _position_meas_local = _position_meas.Get();
        _velocity_desi_local = _velocity_desi.Get();
        
        // initialize
        for (unsigned int i=0; i<_num_axes; i++){
            if (!_is_initialized[i]){
                _is_initialized[i] = true;
                _position_desi_local[i] = _position_meas_local[i];
                _time_begin[i] = TimeService::Instance()->getTicks();
            }
        }
        // position feedback on integrated velocity
        
        for(unsigned int i=0; i<_num_axes; i++){
            double time_difference = TimeService::Instance()->secondsSince(_time_begin[i]);
            _position_desi_local[i] += _velocity_desi_local[i] * time_difference;
            _velocity_out_local[i] = (_controller_gain.value()[i] * (_position_desi_local[i] - _position_meas_local[i])) + _velocity_desi_local[i];
            _time_begin[i] = TimeService::Instance()->getTicks();
        }
        _velocity_out.Set(_velocity_out_local);
    }
    
    void nAxesControllerVel::shutdown()
    {
        for(unsigned int i=0; i<_num_axes; i++){
            _velocity_out_local[i] = 0.0;
        }
        _velocity_out.Set(_velocity_out_local);
    }
    
    void nAxesControllerVel::resetAll()
    {
        for(unsigned int i=0; i<_num_axes; i++)
            _is_initialized[i] = false;
    }
    
    void nAxesControllerVel::resetAxis(int axis)
    {
        _is_initialized[axis] = false;
    }
}//namespace


