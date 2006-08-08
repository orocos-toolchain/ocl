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

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Ports.hpp>

#include <kdl/kdl.hpp>
#include <kdl/frames.hpp>
#include <kdl/kinfam/kinematicfamily.hpp>

namespace Orocos
{
    
    class CartesianEffectorVel : public RTT::GenericTaskContext
    {
    public:
        CartesianEffectorVel(std::string name, KDL::KinematicFamily* kf);
        
        virtual ~CartesianEffectorVel();
        
        virtual bool startup();
        virtual void update();
        virtual void shutdown();
  
    private:
        
        JointVector                                _velocity_joint_local, _position_joint_local;
        KDL::Twist                                 _velocity_cartesian_local;
        KDL::Frame                                 _position_cartesian_local;
        RTT::ReadDataPort< KDL::Twist >            _velocity_cartesian;
        RTT::ReadDataPort< KDL::Frame >            _position_cartesian;
        RTT::ReadDataPort< std::vector<double> >   _position_joint;
        std::vector<RTT::WriteDataPort<double>*>   _velocity_drives;
        
        KDL::KinematicFamily*                      _kf;
        KDL::CartVel2Jnt*                          _cartvel2jnt;
        
    }; // class
}//namespace
#endif // __N_AXES_EFFECTOR_CARTESIAN_VEL_H__
