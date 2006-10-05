// $Id: nAxisGeneratorVel.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
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

#ifndef __N_AXES_GENERATOR_VEL_H__
#define __N_AXES_GENERATOR_VEL_H__

#include <rtt/RTT.hpp>

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>

#include <kdl/motion/velocityprofile_trap.h>
#include <rtt/TimeService.hpp>

namespace Orocos
{
    
    class nAxesGeneratorVel : public RTT::GenericTaskContext
    {
    public:
        nAxesGeneratorVel(std::string name,unsigned int num_axes, 
                          std::string propertyfile="cpf/nAxesGeneratorVel.cpf");
        virtual ~nAxesGeneratorVel();
        
        virtual bool startup();
        virtual void update();
        virtual void shutdown();
        
    private:
        bool setInitVelocity(const int axis, const double velocity);
        bool setInitVelocities(const std::vector<double>& velocity);
        bool velocityFinished(const int axis) const;
        bool velocitiesFinished() const;
        bool applyVelocity(const int axis, const double velocity, double duration=0);
        bool applyVelocities(const std::vector<double>& velocity, double duration=0);
        bool gotoVelocity(const int axis, const double velocity, double duration=0);
        bool gotoVelocities(const std::vector<double>& velocity, double duration=0);
        
        unsigned int                               _num_axes;
        std::string                                _propertyfile;
        
        std::vector<double>                        _duration_desired, _duration_trajectory, _velocity_out_local;
        std::vector<bool>                          _maintain_velocity;
        std::vector<RTT::TimeService::ticks>       _time_begin;
        std::vector<RTT::TimeService::Seconds>     _time_passed;
        std::vector<KDL::VelocityProfile_Trap*>    _vel_profile;
    protected:
        /** 
         * Command to apply a certain velocity to an axis for a amount
         * of time. Command stops when duration is passed.
         * 
         * @param axis number of axis
         * @param velocity desired velocity
         * @param duration duration of the movement
         * 
         * @return false if duration < 0, or axis does not exist, true otherwise
         */
        RTT::Command<bool(int,double,double)          _applyVelocity;

        /** 
         * Same as nAxesGeneratorVel::_applyVelocity but for all axes
         * at the same time. The motion is synchronised.
         * 
         * @param velocity vector with desired velocity
         * @param duration duration of the movement
         * 
         * @return false if duration < 0, true otherwise
         */
        RTT::Command<bool(std::vector<double>,double) _applyVelocities;

        /** 
         * Command to accelerate to a certain velocity and maintain
         * this velocity. Command stops when desired velocity is reached.
         * 
         * @param axis number of axis
         * @param velocity desired velocity
         * @param duration minimum duration of acceleration
         * 
         * @return 
         */
        RTT::Command<bool(int,double,double)          _gotoVelocity;

        /** 
         * 
         * 
         * @param velocity 
         * @param duration 
         * 
         * @return 
         */
        RTT::Command<bool(std::vector<double>,double) _gotoVelocities;

        /** 
         * 
         * 
         * @param axis 
         * @param velocity 
         * 
         * @return 
         */
        RTT::Method<void(int,double)                  _setInitVelocity;
        
        /** 
         * 
         * 
         * @param velocity 
         * 
         * @return 
         */
        RTT::Method<void(std::vector<double>)         _setInitVelocities;
        
        RTT::WriteDataPort< std::vector<double> >  _velocity_desi;
        
        
    }; // class
}//namespace

#endif // __N_AXES_GENERATOR_VEL_H__
