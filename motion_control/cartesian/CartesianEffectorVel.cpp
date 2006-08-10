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

#include "CartesianEffectorVel.hpp"
#include <rtt/Logger.hpp>
#include <assert.h>

namespace Orocos
{
    
    using namespace RTT;
    using namespace KDL;
    using namespace std;
    
    
    CartesianEffectorVel::CartesianEffectorVel(string name, 
                                               KinematicFamily* kf)
        : GenericTaskContext(name),
          _velocity_joint_local(kf->nrOfJoints()),
          _velocity_cartesian("CartesianOutputVelocity"),
          _position_joint("nAxesSensorPosition"),
          _velocity_drives(kf->nrOfJoints()),
          _kf(kf),
          _cartvel2jnt(kf->createCartVel2Jnt())
    {
        //Adding ports
        for (int i=0;i<_kf->nrOfJoints();++i) {
            char buf[80];
            sprintf(buf,"driveValue%d",i);
            _velocity_drives[i] = new WriteDataPort<double>(buf,0);
            ports()->addPort(_velocity_drives[i]);
        }
        this->ports()->addPort(&_velocity_cartesian);
        this->ports()->addPort(&_position_joint);
    }
    
    CartesianEffectorVel::~CartesianEffectorVel()
    {
        delete _cartvel2jnt;
    }
    
    bool CartesianEffectorVel::startup()
    {
        return true;
    }
    
    void CartesianEffectorVel::update()
    {
        _cartvel2jnt->setTwist(_velocity_cartesian.Get());
        _cartvel2jnt->evaluate(_position_joint.Get(),_velocity_joint_local);
        
        for (int i=0; i<_kf->nrOfJoints(); i++)
            _velocity_drives[i]->Set(_velocity_joint_local[i]);
    }
    
    void CartesianEffectorVel::shutdown()
    {
    }
}//namespace
  


