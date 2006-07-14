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

#include "nAxesEffectorVel.hpp"
#include <corelib/Logger.hpp>
#include <assert.h>

namespace Orocos
{
  using namespace RTT;
  using namespace std;
  
  nAxesEffectorVel::nAxesEffectorVel(string name,unsigned int num_axes)
    : GenericTaskContext(name),
      _num_axes(num_axes), 
      _velocity_out_local(num_axes),
      _velocity_out("nAxesOutputVelocity"),
      _velocity_drives(num_axes)
  {
    //Creating TaskContext
  
    //Adding ports
    for (int i=0;i<_num_axes;++i) {
      char buf[80];
      sprintf(buf,"driveValue%d",i);
      _velocity_drives[i] = new WriteDataPort<double>(buf);
      ports()->addPort(_velocity_drives[i]);
    }
    this->ports()->addPort(&_velocity_out);
    
  }
  
  
  nAxesEffectorVel::~nAxesEffectorVel(){};
  
  bool nAxesEffectorVel::startup()
  {
  }
  
  void nAxesEffectorVel::update()
  {
    // copy Output to local values
    _velocity_out_local = _velocity_out.Get();
  
    for (unsigned int i=0; i<_num_axes; i++)
      _velocity_drives[i]->Set(_velocity_out_local[i]);
  }
  
  void nAxesEffectorVel::shutdown()
  {
    for (unsigned int i=0; i<_num_axes; i++)
      _velocity_drives[i]->Set(0.0);
  }
}//namespace




