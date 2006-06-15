// $Id: nAxisGeneratorVel.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
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

#ifndef __N_AXES_GENERATOR_VEL_H__
#define __N_AXES_GENERATOR_VEL_H__

#include <corelib/RTT.hpp>

#include <execution/GenericTaskContext.hpp>
#include <corelib/Properties.hpp>
#include <execution/Ports.hpp>
#include <geometry/velocityprofile_trap.h>
#include <corelib/TimeService.hpp>

namespace Orocos
{
  
  class nAxesGeneratorVel : public RTT::GenericTaskContext
  {
  public:
    nAxesGeneratorVel(std::string name,unsigned int num_axes, 
  		    std::string propertyfile="cpf/nAxesGeneratorVel.cpf");
    virtual ~nAxesGeneratorVel();
  
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
    
  private:
    bool setInitVelocity(const int axis, const double velocity);
    bool setInitVelocities(const std::vector<double>& velocity);
    bool velocityFinished(const int axis) const;
    bool velocitiesFinished() const;
    bool applyVelocity(const int axis, const double velocity, double duration=0);
    bool applyVelocities(const std::vector<double>& velocity, const std::vector<double>& duration);
    bool gotoVelocity(const int axis, const double velocity, double duration=0);
    bool gotoVelocities(const std::vector<double>& velocity, const std::vector<double>& duration);
  
    unsigned int                               _num_axes;
    std::string                                _propertyfile;
  
    std::vector<double>                        _duration_desired, _duration_trajectory, _velocity_out_local;
    std::vector<bool>                          _maintain_velocity;
    std::vector<RTT::TimeService::ticks>       _time_begin;
    std::vector<RTT::TimeService::Seconds>     _time_passed;
    std::vector<ORO_Geometry::VelocityProfile_Trap*>    _vel_profile;
    RTT::WriteDataPort< std::vector<double> >  _velocity_desi;
    RTT::Property< std::vector<double> >       _max_acc, _max_jerk;
    
  }; // class
}//namespace

#endif // __N_AXES_GENERATOR_VEL_H__
