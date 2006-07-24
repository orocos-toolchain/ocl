// $Id: nAxisControllerPosVel.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
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

#ifndef __N_AXES_CONTROLLER_POS_VEL_H__
#define __N_AXES_CONTROLLER_POS_VEL_H__

#include <rtt/RTT.hpp>

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Ports.hpp>

namespace Orocos
{
  class nAxesControllerPosVel : public RTT::GenericTaskContext
  {
  public:
    nAxesControllerPosVel(std::string name, unsigned int num_axes, 
  			std::string propertyfile="cpf/nAxesControllerPosVel.cpf");
    virtual ~nAxesControllerPosVel();
    
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
    
  private:
    unsigned int                              _num_axes;
    std::string                               _propertyfile;
    
    std::vector<double>                       _position_meas_local, _position_desi_local, _velocity_desi_local, _velocity_out_local;
    RTT::ReadDataPort< std::vector<double> >  _position_meas, _position_desi, _velocity_desi;
    RTT::WriteDataPort< std::vector<double> > _velocity_out;
  
    RTT::Property< std::vector<double> >      _controller_gain;
  
  }; // class
}//namespace
#endif // __N_AXES_CONTROLLER_POS_VEL_H__
