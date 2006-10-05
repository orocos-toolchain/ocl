// $Id: nAxisGeneratorCartesianVel.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
// Copyright (C) 2003 Klaas Gadeyne <klaas.gadeyne@mech.kuleuven.ac.be>
//                    Wim Meeussen  <wim.meeussen@mech.kuleuven.ac.be>
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

#include "nAxesGeneratorVel.hpp"
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>
#include <assert.h>

namespace Orocos
{
    using namespace RTT;
    using namespace KDL;
    using namespace std;
    typedef nAxesGeneratorVel MyType;
    
    nAxesGeneratorVel::nAxesGeneratorVel(string name,unsigned int num_axes,
                                         string propertyfile)
        : GenericTaskContext(name),
          _num_axes(num_axes), 
          _propertyfile(propertyfile),
          _duration_desired(num_axes),      
          _duration_trajectory(num_axes),      
          _velocity_out_local(num_axes),
          _maintain_velocity(num_axes),
          _time_begin(num_axes),
          _time_passed(num_axes),
          _vel_profile(num_axes),
          _applyVelocity( "applyVelocity", &MyType::applyVelocity,
                          &MyType::velocityFinished, this),
          _applyVelocities( "applyVelocities", &MyType::applyVelocities,
                            &MyType::velocitiesFinished, this),
          _gotoVelocity( "gotoVelocity", &MyType::gotoVelocity,
                         &MyType::velocityFinished, this),
          _gotoVelocities( "gotoVelocities", &MyType::gotoVelocities,
                           &MyType::velocitiesFinished, this),
          _setInitVelocity( "setInitVelocity", &MyType::setInitVelocity, this),
          _setInitVelocities( "setInitVelocities", &MyType::setInitVelocities, this),
          _velocity_desi("nAxesDesiredVelocity"),
          _max_acc("max_acc", "Maximum Acceleration in Trajectory",vector<double>(num_axes,0)),
          _max_jerk("max_jerk", "Maximum Jerk in Trajectory",vector<double>(num_axes,0))
    {
        //Creating TaskContext
        
        //Adding Properties
        this->properties()->addProperty(&_max_acc);
        this->properties()->addProperty(&_max_jerk);
  
        //Adding Ports
        this->ports()->addPort(&_velocity_desi);
    
        //Creating commands
        this->commands()->addCommand( &_applyVelocities,"Set the velocity",
                                      "velocity", "joint velocity for all axes",
                                      "duration", "duration of movement" );
        this->commands()->addCommand( &_applyVelocity,"Set the velocity for one axis",
                                      "axis", "selected axis",
                                      "velocity", "joint velocity for axis",
                                      "duration", "duration of movement" );
        this->commands()->addCommand( &_gotoVelocities,"Set the velocities",
                                      "velocities", "joint velocities for all axes",
                                      "duration", "duration of movement" );
        this->commands()->addCommand( &_gotoVelocity,"Set the velocity for one axis",
                                      "axis", "selected axis",
                                      "velocity", "joint velocity for axis",
                                      "duration", "duration of movement" );
  
        //Creating Methods
        
        this->methods()->addMethod( &_setInitVelocity,"set initial velocity", 
                                    "axis", "axis where to set velocity",
                                    "velocity", "velocity to set" );
        this->methods()->addMethod( &_setInitVelocities,"set initial velocity", 
                                    "velocities", "velocities to set" );
        
        // Instantiate Motion Profiles
        for( unsigned int i=0; i<_num_axes; i++)
            _vel_profile[i] = new VelocityProfile_Trap( 0,0);
        
        if(!readProperties(_propertyfile))
            Logger::log()<<Logger::Error<<"(nAxesGeneratorVel) Reading Properties from "<<_propertyfile<<" failed!!"<<Logger::endl;

    }
    
    nAxesGeneratorVel::~nAxesGeneratorVel()
    {
        for( unsigned int i=0; i<_num_axes; i++)
            // velocity profiles are only instantiated if updateProperties is called.
            if(_vel_profile[i]) delete _vel_profile[i];
    }
    
    bool nAxesGeneratorVel::startup()
    {
        // check size of properties
        if(_max_acc.value().size() != _num_axes || _max_jerk.value().size() != _num_axes)
            return false;
        
        for (unsigned int i=0; i<_num_axes; i++)
            _vel_profile[i]->SetMax(_max_acc.value()[i], _max_jerk.value()[i]);
        
        // by default initial velocity is set to zero
        for (unsigned int i=0; i<_num_axes; i++){
            _velocity_out_local[i] = 0;
            _maintain_velocity[i] = false;
        }
        
        // generate initial trajectory to maintain current velocity
        for (unsigned int i=0; i<_num_axes; i++)
            applyVelocity(i, _velocity_out_local[i], 0.0);
        
        return true;
    }
  
    
    void nAxesGeneratorVel::update()
    {
        for (unsigned int i = 0 ; i < _num_axes ; i++)
            _time_passed[i] = TimeService::Instance()->secondsSince(_time_begin[i]);
        
        for (unsigned int i = 0 ; i < _num_axes ; i++){
            // still moving
            if (_maintain_velocity[i] || _time_passed[i] <= _duration_desired[i] || _duration_desired[i] == 0)
                _velocity_out_local[i] = _vel_profile[i]->Pos( min(_time_passed[i], _duration_trajectory[i]) );
            
            // stop moving if time is up
            else
                applyVelocity(i, 0.0, 0.0);
        }
        
        _velocity_desi.Set(_velocity_out_local);
    }
    
    void nAxesGeneratorVel::shutdown()
    {
    }
    
    
    bool nAxesGeneratorVel::setInitVelocity(const int axis, const double velocity)
    {
        
        if ( axis < 0 || axis >= (int)_num_axes)
            return false;
        else{
            _velocity_out_local[axis] = velocity;
            applyVelocity(axis, _velocity_out_local[axis], 0.0);
            return true;
        }
    }
    
    
    bool nAxesGeneratorVel::setInitVelocities(const vector<double>& velocity)
    {
        assert(velocity.size() == _num_axes);
        
        bool success = true;
        for (unsigned int i=0; i<_num_axes; i++)
            if (!setInitVelocity(i, velocity[i]))
                success = false;
        return success;
    }
    
    bool nAxesGeneratorVel::gotoVelocity(const int axis, const double velocity, double duration)
    {
        if ( duration < 0 || axis < 0 || axis >= (int)_num_axes)   
            return false;
        else{
            _time_begin[axis]        = TimeService::Instance()->getTicks();
            _time_passed[axis]       = 0;
            _maintain_velocity[axis] = true;
            
            // calculate velocity profile
            _vel_profile[axis]->SetProfileDuration(_velocity_out_local[axis], velocity, duration);
            
            // get duration of acceleration
            _duration_trajectory[axis] = _vel_profile[axis]->Duration();
            _duration_desired[axis] = _duration_trajectory[axis];
            
            return true;
        }
    }
  
    
    bool nAxesGeneratorVel::gotoVelocities(const vector<double>& velocity, double duration)
    {
        assert(velocity.size() == _num_axes);
  
        bool success = true;
        for (unsigned int i=0; i<_num_axes; i++)
            if (!gotoVelocity(i, velocity[i], duration))
                success = false;
        return success;
    }
  
    bool nAxesGeneratorVel::applyVelocity(const int axis, const double velocity, double duration)
    {
        if ( duration < 0 || axis < 0 || axis >= (int)_num_axes)   
            return false;
        else{
            _time_begin[axis]        = TimeService::Instance()->getTicks();
            _time_passed[axis]       = 0;
            _duration_desired[axis]  = duration;
            _maintain_velocity[axis] = false;
  
            // calculate velocity profile
            _vel_profile[axis]->SetProfile(_velocity_out_local[axis], velocity);
            
            // get duration of acceleration
            _duration_trajectory[axis] = _vel_profile[axis]->Duration();
            
            return true;
        }
    }
    
    
    bool nAxesGeneratorVel::applyVelocities(const vector<double>& velocity, double duration)
    {
        assert(velocity.size() == _num_axes);
        
        bool success = true;
        for (unsigned int i=0; i<_num_axes; i++)
            if (!applyVelocity(i, velocity[i], duration))
                success = false;
        return success;
    }
    
    bool nAxesGeneratorVel::velocityFinished(const int axis) const
    {
        return (_time_passed[axis] > _duration_desired[axis] || _duration_desired[axis] == 0);
    }
  
  
    bool nAxesGeneratorVel::velocitiesFinished() const
    {
        bool returnValue=true;
        for (unsigned int i = 0 ; i < _num_axes ; i++)
            if (! velocityFinished(i))
                returnValue = false;
        
        return returnValue;
    }
}//namespace


