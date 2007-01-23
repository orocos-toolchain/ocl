// $Id: nAxisEffectorCartesianVel.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
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

#ifndef __CARTESIAN_EFFECTOR_VEL_H__
#define __CARTESIAN_EFFECTOR_VEL_H__


#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>

#include <kdl/kdl.hpp>
#include <kdl/frames.hpp>
#include <kdl/kinfam/kinematicfamily.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * This class implements a TaskContext that reads out the
     * output twist dataports of OCL::CartesianControllerPos,
     * OCL::CartesianControllerPosVel or OCL::CartesianControllerVel and
     * converts them to axis output velocities using a
     * KDL::KinematicFamily and puts these output values in the
     * driveValue dataports of an nAxesVelocityController. 
     * 
     */
    class CartesianEffectorVel : public RTT::TaskContext
    {
    public:
        /** 
         * Constructor of the class
         * 
         * @param name name of the TaskContext
         * @param kf pointer to the KDL::KinematicFamily to use for
         * kinematic calculations
         * @param offset optional vector that specifies the twist reference
         * point, relative to the end-effector frame
         * 
         */
        CartesianEffectorVel(std::string name, KDL::KinematicFamily* kf, const KDL::Vector& offset=KDL::Vector::Zero());
        
        virtual ~CartesianEffectorVel();
        
        virtual bool startup();
        virtual void update();
        virtual void shutdown();
  
    private:
        JointVector                                _velocity_joint_local;
    protected:
        /// DataPort containing the output twist, represented in the
        /// base frame with end-effector reference point, shared with
        /// OCL::CartesianControllerPos,
        /// OCL::CartesianControllerPosVel or
        /// OCL::CartesianControllerVel
        RTT::ReadDataPort< KDL::Twist >            _velocity_cartesian;
        /// vector of dataports which read from the
        /// nAxesVelocityController. Default looks for ports with
        /// names positionValue0, positionValue1, ...
        RTT::ReadDataPort< std::vector<double> >   _position_joint;
        /// vector of dataports which write to the
        /// nAxesVelocityController. Default looks for ports with
        /// names driveValue0, driveValue1, ...
        std::vector<RTT::WriteDataPort<double>*>   _velocity_drives;
    private:
        KDL::KinematicFamily*                      _kf;
        KDL::CartVel2Jnt*                          _cartvel2jnt;
        /// vector that specifies the twist reference point, relative to the end-effector frame
        KDL::Vector                                _offset;
        
    }; // class
}//namespace
#endif // __N_AXES_EFFECTOR_CARTESIAN_VEL_H__
