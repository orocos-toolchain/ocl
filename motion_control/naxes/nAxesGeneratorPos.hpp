// $Id: nAxisGeneratorPos.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
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

#ifndef __N_AXES_GENERATOR_POS_H__
#define __N_AXES_GENERATOR_POS_H__

#include <rtt/RTT.hpp>

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Ports.hpp>
#include <geometry/velocityprofile_trap.h>
#include <rtt/TimeService.hpp>

namespace Orocos
{
  
  class nAxesGeneratorPos : public RTT::GenericTaskContext
  {
  public:
    
    nAxesGeneratorPos(std::string name,unsigned int num_axes, 
  		    std::string propertyfile="cpf/nAxesGeneratorPos.cpf");
    virtual ~nAxesGeneratorPos();
  
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
  
  private:
    bool moveTo(const std::vector<double>& position, double time=0);
    bool moveFinished() const;
    void reset();
  
    unsigned int                              _num_axes;
    std::string                               _propertyfile;
    
    std::vector<double>                       _position_meas_local,  _position_desi_local, _velocity_desi_local;
    RTT::ReadDataPort< std::vector<double> >  _position_meas;
    RTT::WriteDataPort< std::vector<double> > _position_desi, _velocity_desi;
    
    std::vector<ORO_Geometry::VelocityProfile_Trap*>    _motion_profile;
    RTT::TimeService::ticks                   _time_begin;
    RTT::TimeService::Seconds                 _time_passed;
    double                                    _max_duration;
    
    bool                                      _is_moving;
    RTT::Property< std::vector<double> >      _maximum_velocity, _maximum_acceleration;
  
    
  
  }; // class
}//namespace
#endif // __N_AXES_GENERATOR_POS_H__
