// $Id: nAxisControllerPos.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
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

#ifndef __N_AXES_CONTROLLER_POS_H__
#define __N_AXES_CONTROLLER_POS_H__

#include <corelib/RTT.hpp>

#include <execution/GenericTaskContext.hpp>
#include <corelib/Properties.hpp>
#include <corelib/TimeService.hpp>
#include <execution/Ports.hpp>

namespace Orocos
{
  class nAxesControllerPos : public RTT::GenericTaskContext
  {
  public:
    
    nAxesControllerPos(std::string name, unsigned int num_axes,
		       std::string propertyfile="cpf/nAxesControllerPos.cpf");
    
    virtual ~nAxesControllerPos();
  
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
  
  private:
    bool startMeasuringOffsets(double treshold_moving, int num_samples);
    bool finishedMeasuringOffsets() const;
    const std::vector<double>& getMeasurementOffsets();
    
    unsigned int                                _num_axes;
    std::string                                 _propertyfile;
    
    std::vector<double>                         _position_meas_local, _position_desi_local, _velocity_out_local, _offset_measurement;
    
    RTT::ReadDataPort< std::vector<double> >    _position_meas,  _position_desi;
    RTT::WriteDataPort< std::vector<double> >   _velocity_out;
    int                                         _num_samples, _num_samples_taken;
    double                                      _time_sleep;
    RTT::TimeService::ticks                     _time_begin;
    bool                                        _is_measuring;
    RTT::Property< std::vector<double> >        _controller_gain;
      
  }; // class
}//namespace
#endif // __N_AXES_CONTROLLER_POS_H__
