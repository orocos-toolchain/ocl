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

#include "CartesianSensor.hpp"
#include <rtt/Logger.hpp>
#include <assert.h>

namespace OCL
{
    using namespace RTT;
    using namespace std;
    using namespace KDL;
    
  
    CartesianSensor::CartesianSensor(string name,
                                     KinematicFamily* kf,
				     const KDL::Frame& offset)
        : nAxesSensor(name,kf->nrOfJoints()),
          _frame("CartesianSensorPosition"),
          _twist("CartesianSensorVelocity"),
          _kf(kf),
          _jnt2cartvel(kf->createJnt2CartVel()),
	  _offset(offset)
    {
        ports()->addPort(&_frame);
        ports()->addPort(&_twist);
    }
    
    CartesianSensor::~CartesianSensor(){
        delete _jnt2cartvel;
    };
    
    bool CartesianSensor::startup()
    {
        bool succes = true;

        //initialize values
        succes &= nAxesSensor::startup();
        
        _jnt2cartvel->evaluate(_position_local,_velocity_local);
        _jnt2cartvel->getFrameVel(_FV_local);
    
        _frame.Set(_FV_local.value() * _offset);
        _twist.Set(_FV_local.deriv().RefPoint(_offset.p));
        
        return succes;
    }
  
  
    void CartesianSensor::update()
    {
        nAxesSensor::update();
        
        _jnt2cartvel->evaluate(_position_local,_velocity_local);
        _jnt2cartvel->getFrameVel(_FV_local);
        
        _frame.Set(_FV_local.value() * _offset);
        _twist.Set(_FV_local.deriv().RefPoint(_offset.p));
        
    }
  
    void CartesianSensor::shutdown()
    {
        nAxesSensor::shutdown();
    }
    
}//namespace


