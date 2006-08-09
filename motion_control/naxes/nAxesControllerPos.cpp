// $Id: nAxisGeneratorCartesianPos.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
// Copyright (C) 2003 Klaas Gadeyne <klaas.gadeyne@mech.kuleuven.ac.be>
//                    Wim Meeussen  <wim.meeussen@mech.kuleuven.ac.be>
// Copyright (C) 2006 Ruben Smits <ruben.smits@mech.kuleuven.ac.be>
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


#include "nAxesControllerPos.hpp"
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>
#include <assert.h>

namespace Orocos
{
  
  using namespace RTT;
  using namespace std;
  
  nAxesControllerPos::nAxesControllerPos(string name,unsigned int num_axes, 
  				       string propertyfile)
    : GenericTaskContext(name),
      _num_axes(num_axes), 
      _propertyfile(propertyfile),
      _position_meas_local(num_axes),
      _position_desi_local(num_axes),
      _velocity_out_local(num_axes),
      _offset_measurement(num_axes),
      _position_meas("nAxesSensorPosition"),
      _position_desi("nAxesDesiredPosition"),
      _velocity_out("nAxesOutputVelocity"),
      _controller_gain("K", "Proportional Gain")
  {
    //Creating TaskContext

    //Adding Ports
    this->ports()->addPort(&_position_meas);
    this->ports()->addPort(&_position_desi);
    this->ports()->addPort(&_velocity_out);
    
    //Adding Properties
    this->properties()->addProperty(&_controller_gain);
    
    //Adding Commands
    typedef nAxesControllerPos MyType;

    this->commands()->addCommand( command( "measureOffset", &MyType::startMeasuringOffsets,
							     &MyType::finishedMeasuringOffsets, this),
				  "calculate the velocity offset on the axes",
				  "time_sleep", "time to wait before starting measurement",
				  "num_samples", "number of samples to take");
    //Adding Methods

    this->methods()->addMethod( method( "getOffset", &MyType::getMeasurementOffsets, this),
				"Get offset measurements");

    if(!readProperties(_propertyfile)){
      Logger::log()<<Logger::Error<<"(nAxesControllerPos) Reading Properties from "<<_propertyfile<<" failed!!"<<Logger::endl;
    }

  }
  
  nAxesControllerPos::~nAxesControllerPos(){};
  
  bool nAxesControllerPos::startup()
  {
    
    // check size of properties
    if(_controller_gain.value().size() != _num_axes)
      return false;
    
    
    //Initialize
    _is_measuring = false;
    
    return true;
    
  }
  
  
  void nAxesControllerPos::update()
  {
    // copy Input and Setpoint to local values
    _position_meas_local = _position_meas.Get();
    _position_desi_local = _position_desi.Get();
  
    // position feedback
    for(unsigned int i=0; i<_num_axes; i++)
      _velocity_out_local[i] = _controller_gain.value()[i] * (_position_desi_local[i] - _position_meas_local[i]);
  
    // measure offsets
    if (_is_measuring && TimeService::Instance()->secondsSince(_time_begin) > _time_sleep){
      for (unsigned int i=0; i<_num_axes; i++)
  	_offset_measurement[i] += _velocity_out_local[i] / _num_samples;
      _num_samples_taken++;
      if (_num_samples_taken == _num_samples)  _is_measuring = false;
    }
    _velocity_out.Set(_velocity_out_local);
  }
  
  void nAxesControllerPos::shutdown()
  {
    for(unsigned int i=0; i<_num_axes; i++){
		_velocity_out_local[i] = 0.0;
	}
	_velocity_out.Set(_velocity_out_local);
  }
  
  bool nAxesControllerPos::startMeasuringOffsets(double time_sleep, int num_samples)
  {
    Logger::log()<<Logger::Debug<<"(nAxesControllerPos) start measuring offsets"<<Logger::endl;
    
    // don't do anything if still measuring
    if (_is_measuring)
      return false;
    
    // get new measurement
    else{
      for (unsigned int i=0; i<_num_axes; i++){
  	_offset_measurement[i] = 0;
      }
      _time_sleep        = max(1.0, time_sleep);  // min 1 sec
      _time_begin        = TimeService::Instance()->getTicks();
      _num_samples       = max(1,num_samples);    // min 1 sample
      _num_samples_taken = 0;
      _is_measuring      = true;
      return true;
    }
  }
  
  
  bool nAxesControllerPos::finishedMeasuringOffsets() const
  {
    return !_is_measuring;
  }
  
  
  const std::vector<double>& nAxesControllerPos::getMeasurementOffsets()
  {
    return _offset_measurement;
  }
}//namespace



