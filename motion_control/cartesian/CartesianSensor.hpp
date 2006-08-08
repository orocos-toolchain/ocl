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

#include "motion_control/naxes/nAxesSensor.hpp"

#include <kdl/kdl.hpp>
#include <kdl/frames.hpp>
#include <kdl/kinfam/kinematicfamily.hpp>

namespace Orocos
{
    class CartesianSensor : public nAxesSensor
    {
    public:
        CartesianSensor(std::string name,
                        KDL::KinematicFamily* kf);
        
        virtual ~CartesianSensor();
        
        // Redefining virtual members
        virtual bool startup();
        virtual void update();
        virtual void shutdown();
  
    private:
        KDL::FrameVel                      _FV_local;
        RTT::WriteDataPort< KDL::Frame >   _frame;
        RTT::WriteDataPort< KDL::Twist >   _twist;
        
        KDL::KinematicFamily*              _kf;
        //KDL::Jnt2CartPos*                  _jnt2cartpos;
        KDL::Jnt2CartVel*                  _jnt2cartvel;
        
    }; // class
}//namespace
#endif // __CARTESIAN_SENSOR_H__
