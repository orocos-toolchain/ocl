// Copyright (C) 2006 Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
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

#ifndef __CARTESIAN_SENSOR_POS_H__
#define __CARTESIAN_SENSOR_POS_H__

#include "../naxes/nAxesSensor.hpp"

#include <geometry/GeometryToolkit.hpp>

namespace Orocos
{
  class CartesianSensor : public nAxesSensor
  {
  public:
    CartesianSensor(std::string name,unsigned int num_axes,
			  std::string kine_comp_name);
    
    virtual ~CartesianSensor();
  
    // Redefining virtual members
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
  
  private:
    ORO_Geometry::Frame                         _frame_local;
    ORO_Geometry::Twist                         _twist_local;
    RTT::WriteDataPort< ORO_Geometry::Frame >   _frame;
    RTT::WriteDataPort< ORO_Geometry::Twist >   _twist;

    std::string                                 _kine_comp_name;
        
    RTT::MethodC                                _positionForward, _velocityForward;
          
  }; // class
}//namespace
#endif // __CARTESIAN_SENSOR_H__
