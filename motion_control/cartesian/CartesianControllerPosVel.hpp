// $Id: nAxisPosVelController.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
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

#ifndef __CARTESIAN_POS_VEL_CONTROLLER_H__
#define __CARTESIAN_POS_VEL_CONTROLLER_H__

#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Ports.hpp>

#include <kdl/kdl.hpp>
#include <kdl/frames.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * This class implements a TaskContext that controlls the
     * end-effector frame of a robot. It uses a simple
     * position-feedback and velocity-feedforward to calculate an
     * output twist.  
     * twist_out = K_gain * ( frame_desired - frame_measured) + 
     *             twist_desired.
     */
    class CartesianControllerPosVel : public RTT::TaskContext
    {
    public:
        /** 
         * Constructor of the class
         * 
         * @param name name of the TaskContext
         * 
         */
        CartesianControllerPosVel(std::string name);
        virtual ~CartesianControllerPosVel();
       
        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();
        virtual void stopHook();
        virtual void cleanupHook();
        
    private:
        KDL::Frame                  _position_meas_local, _position_desi_local;
        KDL::Twist                  _velocity_out_local, _velocity_desi_local, _velocity_feedback;
        std::vector<double>         _gain_local;
        
    protected:
        /// DataPort containing the measured frame, shared with
        /// OCL::CartesianSensor 
        RTT::ReadDataPort< KDL::Frame >    _position_meas;
        /// DataPort containing the desired frame, shared with
        /// OCL::CartesianGeneratorPos  
        RTT::ReadDataPort< KDL::Frame >    _position_desi;
        /// DataPort containing the desired twist, represented in the
        /// base frame with end-effector reference point, shared with
        /// OCL::CartesianGeneratorPos
        RTT::ReadDataPort< KDL::Twist >    _velocity_desi;
        /// DataPort containing the output twist, represented in the
        /// base frame with end-effector reference point, shared with
        /// OCL::CartesianEffectorVel
        RTT::WriteDataPort< KDL::Twist >   _velocity_out;
        /// Vector with the control gain value for each dof.
        RTT::Property< std::vector<double> >        _controller_gain;
        
    }; // class
}//namespace
#endif // __N_AXES_CARTESIAN_POS_VEL_CONTROLLER_H__
