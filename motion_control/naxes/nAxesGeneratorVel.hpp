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

#include <kdl/velocityprofile_trap.hpp>
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
     * OCL::nAxesSensor and generates velocity
     * setpoints which can be use by OCL::nAxesControllerVel.
     * 
     * \ingroup nAxesComponents 
     */
    
    class nAxesGeneratorVel : public RTT::TaskContext
    {
    public:
        /** 
         * Constructor of the class
         * 
         * @param name name of the TaskContext
         *
         * @return 
         */
        nAxesGeneratorVel(std::string name);
        virtual ~nAxesGeneratorVel();
        
        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();
        virtual void stopHook();
        
    private:
        bool setInitVelocity(const unsigned int axis, const double velocity);
        bool setInitVelocities(const std::vector<double>& velocity);
        bool velocityFinished(const unsigned int axis) const;
        bool velocitiesFinished() const;
        bool applyVelocity(const unsigned int axis, const double velocity, double duration=0);
        bool applyVelocities(const std::vector<double>& velocity, double duration=0);
        bool gotoVelocity(const int unsigned axis, const double velocity, double duration=0);
        bool gotoVelocities(const std::vector<double>& velocity, double duration=0);
        
        unsigned int                               num_axes;
        
        std::vector<double>                        duration_desired, duration_trajectory, v_d, a_max, j_max;
        std::vector<bool>                          maintain_velocity;
        std::vector<RTT::TimeService::ticks>       time_begin;
        std::vector<RTT::TimeService::Seconds>     time_passed;
        std::vector<KDL::VelocityProfile_Trap>     vel_profile;
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
        RTT::Command<bool(int,double,double)>          applyVelocity_cmd;

        /** 
         * Same as nAxesGeneratorVel::_applyVelocity but for all axes
         * at the same time.
         * 
         * @param velocity vector with desired velocities
         * @param duration duration of the movement
         * 
         * @return false if duration < 0, true otherwise
         */
        RTT::Command<bool(std::vector<double>,double)> applyVelocities_cmd;

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
        RTT::Command<bool(int,double,double)>          gotoVelocity_cmd;

        /** 
         * Same as nAxesGeneratorVel::_gotoVelocity but for all axes
         * at the same time.

         * 
         * @param velocity vector with desired velocities
         * @param duration minimum duration of the acceleration
         * 
         * @return false if duration<0, true otherwise
         */
        RTT::Command<bool(std::vector<double>,double)> gotoVelocities_cmd;

        /** 
         * Maybe Wim should explain what this Method does
         * 
         * @param axis 
         * @param velocity 
         * 
         * @return 
         */
        RTT::Method<void(int,double)>                  setInitVelocity_mtd;
        
        /** 
         * Same as nAxesGeneratorVel::_setInitVelocity but for all
         * axes at the same time
         * 
         * @param velocity 
         * 
         * @return 
         */
        RTT::Method<void(std::vector<double>)>         setInitVelocities_mtd;

        /// DataPort containing the current desired velocities, shared
        /// with OCL::nAxesControllerVel 
        RTT::WriteDataPort< std::vector<double> >     v_d_port;
        /// Property with a vector of the maximum accelerations of
        /// each axes to be used in the interpolation
        RTT::Property<std::vector<double> >           a_max_prop;
        /// Property with a vector of the maximum jerks of
        /// each axes to be used in the interpolation
        RTT::Property<std::vector<double> >           j_max_prop;
        RTT::Property< unsigned int > num_axes_prop; 
        
    }; // class
}//namespace

#endif // __N_AXES_GENERATOR_VEL_H__
