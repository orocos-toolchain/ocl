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

#include "CartesianGeneratorPos.hpp"
#include <assert.h>
#include <ocl/ComponentLoader.hpp>

ORO_LIST_COMPONENT_TYPE( OCL::CartesianGeneratorPos );

namespace OCL
{

    using namespace RTT;
    using namespace KDL;
    using namespace std;

    CartesianGeneratorPos::CartesianGeneratorPos(string name)
        : TaskContext(name,PreOperational),
          _maximum_velocity_local(6,0.0),
          _maximum_acceleration_local(6,0.0),
          _motion_profile(6,VelocityProfile_Trap(0,0)),
          _is_moving(false),
          _moveTo( "moveTo", &CartesianGeneratorPos::moveTo,
                   &CartesianGeneratorPos::moveFinished, this),
          _reset_position( "resetPosition", &CartesianGeneratorPos::resetPosition, this),
          _position_meas("CartesianSensorPosition"),
          _position_desi("CartesianDesiredPosition"),
          _velocity_desi("CartesianDesiredVelocity"),
          _maximum_velocity("max_vel", "Maximum Velocity in Trajectory",vector<double>(6,0.0)),
          _maximum_acceleration("max_acc", "Maximum Acceleration in Trajectory",vector<double>(6,0.0))
    {
        //Creating TaskContext

        //Adding Ports
        this->ports()->addPort(&_position_meas);
        this->ports()->addPort(&_position_desi);
        this->ports()->addPort(&_velocity_desi);

        //Adding Properties
        this->properties()->addProperty(&_maximum_velocity);
        this->properties()->addProperty(&_maximum_acceleration);

        //Adding Commands
        this->commands()->addCommand( &_moveTo,"Set the position setpoint",
                                      "setpoint", "position setpoint for end effector",
                                      "time", "minimum time to execute trajectory" );

        //Adding Methods
        this->methods()->addMethod( &_reset_position, "Reset generator's position" );

    }

    CartesianGeneratorPos::~CartesianGeneratorPos()
    {
    }

    bool CartesianGeneratorPos::configureHook()
    {
        //        if(!marshalling()->readProperties(this->getName()+".cpf"))
        //    return false;
        //Check if sizes are correct
        if(_maximum_acceleration.value().size()!=6||_maximum_velocity.value().size()!=6)
            return false;
        //copy property values in local variable
        _maximum_velocity_local=_maximum_velocity.value();
        _maximum_acceleration_local=_maximum_acceleration.value();

        for(unsigned int i=0;i<6;i++)
            _motion_profile[i].SetMax(_maximum_velocity_local[i],_maximum_acceleration_local[i]);

        return true;

    }

    bool CartesianGeneratorPos::startHook()
    {
        _is_moving = false;

        //initialize
        _position_desi_local = _position_meas.Get();
        SetToZero(_velocity_desi_local);
        _position_desi.Set(_position_desi_local);
        _velocity_desi.Set(_velocity_desi_local);

        return true;
    }

    void CartesianGeneratorPos::updateHook()
    {
        if (_is_moving){
            _time_passed = TimeService::Instance()->secondsSince(_time_begin);
            if ( _time_passed > _max_duration ){
                // set end position
                _position_desi_local = _traject_end;
                SetToZero(_velocity_desi_local);
                _is_moving = false;
            }else{
                // position
                _velocity_delta = Twist(Vector(_motion_profile[0].Pos(_time_passed),_motion_profile[1].Pos(_time_passed),_motion_profile[2].Pos(_time_passed)),
                                        Vector(_motion_profile[3].Pos(_time_passed),_motion_profile[4].Pos(_time_passed),_motion_profile[5].Pos(_time_passed)) );
                _position_desi_local = Frame( _traject_begin.M * Rot( _traject_begin.M.Inverse( _velocity_delta.rot ) ), _traject_begin.p + _velocity_delta.vel);

                // velocity
                for(unsigned int i=0; i<6; i++)
                    _velocity_desi_local(i) = _motion_profile[i].Vel( _time_passed );
            }
            _position_desi.Set(_position_desi_local);
            _velocity_desi.Set(_velocity_desi_local);
        }
    }

    void CartesianGeneratorPos::stopHook()
    {
    }

    void CartesianGeneratorPos::cleanupHook()
    {
    }

    bool CartesianGeneratorPos::moveTo(Frame frame, double time)
    {

        // if previous movement is finished
        if (!_is_moving){
            _max_duration = 0;
            _traject_end = frame;

            // get current position
            _traject_begin = _position_meas.Get();
            _velocity_begin_end = diff(_traject_begin, _traject_end);

            // Set motion profiles
            for (unsigned int i=0; i<6; i++){
                _motion_profile[i].SetProfileDuration( 0, _velocity_begin_end(i), time );
                _max_duration = max( _max_duration, _motion_profile[i].Duration() );
            }

            // Rescale trajectories to maximal duration
            for (unsigned int i=0; i<6; i++)
                _motion_profile[i].SetProfileDuration( 0, _velocity_begin_end(i), _max_duration );

            _time_begin = TimeService::Instance()->getTicks();
            _time_passed = 0;

            _is_moving = true;
            return true;
        }
        else//still moving
            return false;
    }

    bool CartesianGeneratorPos::moveFinished() const
    {
        return (!_is_moving );
    }


    void CartesianGeneratorPos::resetPosition()
    {
        _position_desi_local = _position_meas.Get();
        SetToZero(_velocity_desi_local);
        _position_desi.Set(_position_desi_local);
        _velocity_desi.Set(_velocity_desi_local);
        _is_moving = false;
    }
}//namespace



