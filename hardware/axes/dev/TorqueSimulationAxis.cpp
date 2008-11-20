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
#include <limits>

using namespace OCL;

TorqueSimulationEncoder::TorqueSimulationEncoder(double initial, double min, double max):
    _position(initial),
    _min(min),
    _max(max),
    _first_drive(true)
{}

double
TorqueSimulationEncoder::readSensor() const
{
  if (_first_drive){
    return _position;
  }

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
TorqueSimulationEncoder::update(double position, double velocity, TimeService::ticks previous_time)
{
  if (_first_drive)
    _first_drive = false;

  _position = position;
  _velocity = velocity;
  _previous_time = previous_time;
}

void
TorqueSimulationEncoder::stop()
{
  _velocity = 0.0;
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
  if (_first_drive){
    return _velocity;
  }

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


void
TorqueSimulationVelocitySensor::update(double velocity, double acceleration, TimeService::ticks previous_time)
{
  if (_first_drive)
    _first_drive = false;

  _velocity = velocity;
  _acceleration = acceleration;
  _previous_time = previous_time;
}

void
TorqueSimulationVelocitySensor::stop()
{
  _velocity = 0.0;
  _acceleration = 0.0;
}

TorqueSimulationAxis::TorqueSimulationAxis(double initial, double min, double max, double velLim):
  _drive_value(0),
  _enable(false), _brake(true),
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
TorqueSimulationAxis::drive( double current )
{
    // detect enable switch
    if ( !_enable.isOn() )
        return false;
    // detect if brake is on.
    if ( _brake.isOn() )
        return false;

 if (_is_stopped || _is_driven){
    if ( (current < -_max_drive_value) || (current > _max_drive_value) ){
      //std::cerr << "(TorqueSimulationAxis)  Maximum drive value exceeded. Axis.disable()" << std::endl;
      stop();
      lock();
      return false;
    }
    else{
        _drive_value = current;
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
    _encoder->stop();
    _velSensor->stop();
    _drive_value = 0.0;
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

