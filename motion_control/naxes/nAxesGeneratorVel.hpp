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

#include <rtt/TaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>

#include <kdl/motion/velocityprofile_trap.h>
#include <rtt/TimeService.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * This class implements a TaskContext that generates a path
     * between the current velocities and the new desired velocities of a
     * number of axes, this velocity can be maintained or stopped.
     * It uses KDL for the time interpolation. The
     * interpolation uses a trapezoidal acceleration profile using a
     * maximum acceleration and a maximum jerk. It takes the
     * current velocity from a dataport shared with
     * Orocos::nAxesSensor and generates velocity
     * setpoints which can be use by Orocos::nAxesControllerVel.
     * 
     */
    
    class nAxesGeneratorVel : public RTT::TaskContext
    {
    public:
        /** 
         * Constructor of the class
         * 
         * @param name name of the TaskContext
         * @param num_axes number of axes
         * @param propertyfile location of propertyfile. Default:
         * cpf/nAxesGeneratorVel.cpf 
         * 
         * @return 
         */
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
        RTT::Command<bool(int,double,double)>          _applyVelocity;

        /** 
         * Same as nAxesGeneratorVel::_applyVelocity but for all axes
         * at the same time.
         * 
         * @param velocity vector with desired velocities
         * @param duration duration of the movement
         * 
         * @return false if duration < 0, true otherwise
         */
        RTT::Command<bool(std::vector<double>,double)> _applyVelocities;

        /** 
         * Command to accelerate to a certain velocity and maintain
         * this velocity. Command stops when desired velocity is reached.
         * 
         * @param axis number of axis
         * @param velocity desired velocity
         * @param duration minimum duration of acceleration
         * 
         * @return false if axis does not exist or duration < 0, true
         *otherwise 
         */
        RTT::Command<bool(int,double,double)>          _gotoVelocity;

        /** 
         * Same as nAxesGeneratorVel::_gotoVelocity but for all axes
         * at the same time.

         * 
         * @param velocity vector with desired velocities
         * @param duration minimum duration of the acceleration
         * 
         * @return false if duration<0, true otherwise
         */
        RTT::Command<bool(std::vector<double>,double)> _gotoVelocities;

        /** 
         * Maybe Wim should explain what this Method does
         * 
         * @param axis 
         * @param velocity 
         * 
         * @return 
         */
        RTT::Method<void(int,double)>                  _setInitVelocity;
        
        /** 
         * Same as nAxesGeneratorVel::_setInitVelocity but for all
         * axes at the same time
         * 
         * @param velocity 
         * 
         * @return 
         */
        RTT::Method<void(std::vector<double>)>         _setInitVelocities;

        /// DataPort containing the current desired velocities, shared
        /// with Orocos::nAxesControllerVel 
        RTT::WriteDataPort< std::vector<double> >     _velocity_desi;
        /// Property with a vector of the maximum accelerations of
        /// each axes to be used in the interpolation
        RTT::Property<std::vector<double> >           _max_acc;
        /// Property with a vector of the maximum jerks of
        /// each axes to be used in the interpolation
        RTT::Property<std::vector<double> >           _max_jerk;
        
    }; // class
}//namespace

#endif // __N_AXES_GENERATOR_VEL_H__
