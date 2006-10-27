/***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#include "TorqueSimulationAxis.hpp"
#include <rtt/Logger.hpp>

using namespace RTT;

TorqueSimulationEncoder::TorqueSimulationEncoder(double initial, double min, double max):
    _position(initial),
    _min(min),
    _max(max),
    _first_drive(true)
{}
  
double 
TorqueSimulationEncoder::readSensor() const
{
  if (_first_drive)
    return _position;

  else{
    // get new position, using time and velocity
    TimeService::Seconds _delta_time = TimeService::Instance()->secondsSince(_previous_time);
    return _position + (_velocity*(double)_delta_time);
  }
}

int 
TorqueSimulationEncoder::readSensor(double& data) const
{
  data = readSensor();
  return 0;
}


void 
TorqueSimulationEncoder::setDrive(double velocity) 
{
  // adjust position, using previous velocity
  if (_first_drive)
    _first_drive = false;

  else{
    _delta_time = TimeService::Instance()->secondsSince(_previous_time);
    _position += _velocity*(double)_delta_time;
  }
  
  // set new velocity and start time
  _previous_time = TimeService::Instance()->getTicks();
  _velocity = velocity;
}









TorqueSimulationVelocitySensor::TorqueSimulationVelocitySensor( double maxvel):
    _velocity(0),
    _acceleration(0),
    _maxvel(maxvel),
    _first_drive(true)
{}
  
double 
TorqueSimulationVelocitySensor::readSensor() const
{
  if (_first_drive)
    return _velocity;

  else{
    // get new position, using time and velocity
    TimeService::Seconds _delta_time = TimeService::Instance()->secondsSince(_previous_time);
    return _velocity + (_acceleration*(double)_delta_time);
  }
}

int
TorqueSimulationVelocitySensor::readSensor(double& data) const
{
  data = readSensor();
  return 0;
}


double 
TorqueSimulationVelocitySensor::setDrive(double acceleration) 
{
  // adjust velocity, using previous acceleration
  if (_first_drive){
    _first_drive = false;
  }
  else{
    _delta_time = TimeService::Instance()->secondsSince(_previous_time);
    _velocity += _acceleration*(double)_delta_time;
  }
  
  // set new acceleration and start time
  _previous_time = TimeService::Instance()->getTicks();
  _acceleration = acceleration;
return _velocity;
}












TorqueSimulationAxis::TorqueSimulationAxis(double initial, double min, double max, double velLim):
  _drive_value(0),
  _enable(false), _brake(true),
  _velocity(0),
  _max_drive_value(std::numeric_limits<double>::max()),
  _encoder( new TorqueSimulationEncoder( initial, min, max) ),
  _velSensor( new TorqueSimulationVelocitySensor( velLim ) ),
  _curSensor( new TorqueSimulationCurrentSensor( this, _max_drive_value ) ),
  _is_locked(true),
  _is_stopped(false),
  _is_driven(false)
{}



TorqueSimulationAxis::~TorqueSimulationAxis()
{
  delete _encoder;
  delete _velSensor;
  delete _curSensor;
}

bool 
TorqueSimulationAxis::drive( double cur )
{
    //this method should only be used to start the axis, to arguments are necessary to do a simulation
    if (cur!=0){
	Logger::log()<<Logger::Error<<"Use drive_sim(double, double) in simulation mode"<<Logger::endl;
    }
    return false;
}

bool 
TorqueSimulationAxis::drive_sim( double cur, double acc )
{ 
    // detect enable switch
    if ( !_enable.isOn() )
        return false;
    // detect if brake is on.
    if ( _brake.isOn() )
        return false;

 if (_is_stopped || _is_driven){
    if ( (acc < -_max_drive_value) || (acc > _max_drive_value) ){
      //std::cerr << "(TorqueSimulationAxis)  Maximum drive value exceeded. Axis.disable()" << std::endl;
      stop();
      lock();
      return false;
    }
    else{
	//Logger::log()<<Logger::Debug<<"_acceleration "<< acc <<Logger::endl;
       _velocity = _velSensor->setDrive(acc);
       _encoder->setDrive(_velocity);
       _drive_value = cur;
       _is_stopped = false;
       _is_driven  = true;
      return true;
   }
 }
 else
   return false;
}

bool
TorqueSimulationAxis::stop()
{
  if (_is_driven){
    _encoder->setDrive(0);
    _velSensor->setDrive(0);
    _drive_value = 0;
    _is_driven  = false;
    _is_stopped = true;
    return true;
  }
  else if (_is_stopped)
    return true;
  else
    return false;
}
  
bool
TorqueSimulationAxis::lock()
{
  if (_is_stopped){
    _is_locked  = true;
    _is_stopped = false;
    _brake.switchOn();
    _enable.switchOff();
    return true;
  }
  else if (_is_locked)
    return true;
  else
    return false;
}
      
bool
TorqueSimulationAxis::unlock()
{
  if (_is_locked){
    _is_locked  = false;
    _is_stopped = true;
    _brake.switchOff();
    _enable.switchOn();
    return true;
  }
  else if (_is_stopped)
    return true;
  else
    return false;
}

bool
TorqueSimulationAxis::isLocked() const
{
  return _is_locked;
}

bool
TorqueSimulationAxis::isStopped() const
{
  return _is_stopped;
}

bool
TorqueSimulationAxis::isDriven() const
{
  return _is_driven;
}

DigitalOutput* TorqueSimulationAxis::getBrake()
{ 
    return &_brake; 
}

DigitalOutput* TorqueSimulationAxis::getEnable()
{ 
    return &_enable; 
}



SensorInterface<double>* 
TorqueSimulationAxis::getSensor(const std::string& name) const
{
  if (name == "Position")
      return _encoder;
  if (name == "Velocity")
      return _velSensor;
  if (name == "Current")
      return _curSensor;

  return NULL;
}


std::vector<std::string> 
TorqueSimulationAxis::sensorList() const
{
  std::vector<std::string> result;
  result.push_back("Position");
  result.push_back("Velocity");
  result.push_back("Current");
  return result;
}

double TorqueSimulationAxis::getDriveValue() const
{
  return _drive_value;
}

