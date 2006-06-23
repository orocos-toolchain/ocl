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
#include <corelib/Logger.hpp>
#include <assert.h>

namespace Orocos
{
    
  using namespace RTT;
  using namespace ORO_Geometry;
  using namespace std;
  
  
  CartesianEffectorVel::CartesianEffectorVel(string name,unsigned int num_axes, 
					     string kine_comp_name)
    : GenericTaskContext(name),
      _num_axes(num_axes),
      _velocity_joint_local(num_axes),
      _position_joint_local(num_axes),
      _velocity_cartesian("CartesianOutputVelocity"),
      _position_cartesian("CartesianSensorPosition"),
      _position_joint("nAxesSensorPosition"),
      _velocity_drives(num_axes),
      _kine_comp_name(kine_comp_name)
  {
    Toolkit::Import( GeometryToolkit );
    
    //Adding ports
    for (int i=0;i<_num_axes;++i) {
      char buf[80];
      sprintf(buf,"driveValue%d",i);
      _velocity_drives[i] = new WriteDataPort<double>(buf);
      ports()->addPort(_velocity_drives[i]);
    }
    this->ports()->addPort(&_velocity_cartesian);
    this->ports()->addPort(&_position_cartesian);
    this->ports()->addPort(&_position_joint);
  }
  
  
  CartesianEffectorVel::~CartesianEffectorVel(){};
  
  bool CartesianEffectorVel::startup()
  {
    try{
      _velocityInverse = getPeer(_kine_comp_name)->methods()->create("this","velocityInverse").
	arg(_position_joint_local).arg(_velocity_cartesian_local).arg(_velocity_joint_local);
    }
    catch(...){
      return false;
    }
    //Initialize
    _position_cartesian_local = _position_cartesian.Get();
    SetToZero(_velocity_cartesian_local);
    _position_joint_local = _position_joint.Get();

    return _velocityInverse.execute();
  }
  
  void CartesianEffectorVel::update()
  {
    // copy to local values
    _velocity_cartesian_local = _velocity_cartesian.Get().RefPoint(_position_cartesian_local.p * -1);
    _position_cartesian_local = _position_cartesian.Get();
    _position_joint_local = _position_joint.Get();
    
    // inverse velocity kinematics
    _velocityInverse.execute();

    for (unsigned int i=0; i<_num_axes; i++)
      _velocity_drives[i]->Set(_velocity_joint_local[i]);
  }
  
  void CartesianEffectorVel::shutdown()
  {
  }
}//namespace
  


