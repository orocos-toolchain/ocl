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

#ifndef __CARTESIAN_GENERATOR_POS_H__
#define __CARTESIAN_GENERATOR_POS_H__

#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>

#include <kdl/velocityprofile_trap.hpp>
#include <rtt/TimeService.hpp>

#include <kdl/kdl.hpp>
#include <kdl/frames.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * This class implements a TaskContext that creates a path in
     * Cartesian space between the current cartesian position and a
     * new desired cartesian position. It uses trapezoidal
     * velocity-profiles for every dof using a maximum velocity and a
     * maximum acceleration. It generates frame and twist setpoints
     * which can be used by OCL::CartesianControllerPos,
     * OCL::CartesianControllerPosVel or OCL::CartesianControllerVel.
     * 
     */
    class CartesianGeneratorPos : public RTT::TaskContext
    {
    public:
        /** 
         * Constructor of the class.
         * 
         * @param name name of the TaskContext
         * @param propertyfile location of the propertyfile. Default:
         * cpf/CartesianGeneratorPos.cpf 
         * 
         */
        CartesianGeneratorPos(std::string name,std::string propertyfile="cpf/CartesianGeneratorPos.cpf");
        virtual ~CartesianGeneratorPos();
        
        virtual bool startup();
        virtual void update();
        virtual void shutdown();
  
    private:
        bool moveTo(KDL::Frame frame, double time=0);
        bool moveFinished() const;
        void resetPosition();
  
        const std::string                 _propertyfile;
        
        KDL::Frame                        _traject_end, _traject_begin;
        KDL::Frame                        _position_desi_local;
        KDL::Twist                        _velocity_desi_local, _velocity_begin_end, _velocity_delta;

    protected:
        /** 
         * Command to generate the motion. Command stops when the
         * movement is finished.
         * 
         * @param frame the desired frame
         * @param time the minimum time duration of the movement.
         * 
         * @return false if a previous motion is still going on, true otherwise
         */
        RTT::Command<bool(KDL::Frame,double)> _moveTo;
        
        /**
         * Method that resets the generators desired frame tho the
         *current measured frame and the desired twist to zero.
         */
        RTT::Method<void(void)>           _reset_position;
        /// Dataport containing the current measured end-effector
        /// frame, shared with OCL::CartesianSensor
        RTT::ReadDataPort< KDL::Frame >   _position_meas;
        /// Dataport containing the current desired end-effector
        /// frame, shared with OCL::CartesianControllerPos,
        /// OCL::CartesianControllerPosVel 
        RTT::WriteDataPort< KDL::Frame >  _position_desi;
        /// Dataport containing the current desired end-effector
        /// twist, shared with OCL::CartesianControllerPosVel,
        /// OCL::CartesianControllerVel 
        RTT::WriteDataPort< KDL::Twist >  _velocity_desi;
        /// Property containing a vector with the maximum velocity of
        /// each dof
        RTT::Property< std::vector<double> >  _maximum_velocity;
        /// Property containing a vector with the maximum acceleration of
        /// each dof
        RTT::Property< std::vector<double> >  _maximum_acceleration;

    private:  
        std::vector<KDL::VelocityProfile_Trap*>     _motion_profile;
        RTT::TimeService::ticks                     _time_begin;
        RTT::TimeService::Seconds                   _time_passed;
        double                                      _max_duration;
        
        bool                                        _is_moving;

    
  }; // class
}//namespace

#endif // __CARTESIAN_GENERATOR_POS_H__
