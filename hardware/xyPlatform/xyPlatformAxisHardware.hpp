/***************************************************************************
  tag: Ruben Smits  Mon Jan 19 14:11:20 CET 2005  JR3WrenchSensor.hpp 

                        JR3WrenchSensor.hpp -  description
                           -------------------
    begin                : Mon January 19 2004
    copyright            : (C) 2004 Peter Soetens
    email                : first.last@mech.kuleuven.ac.be
 
 ***************************************************************************
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


#ifndef XYPLATFORM_AXIS_HARDWARE_HPP
#define XYPLATFORM_AXIS_HARDWARE_HPP

#include <rtt/dev/AnalogOutput.hpp>
#include <rtt/dev/DigitalOutput.hpp>
//#include <rtt/dev/DigitalInput.hpp>
#include <rtt/dev/AxisInterface.hpp>

// Axes classes
#include "dev/AnalogDrive.hpp"
#include "dev/Axis.hpp"
#include "dev/IncrementalEncoderSensor.hpp"

#include <rtt/TaskContext.hpp>
#include "xyPlatformConstants.hpp"

#define XYPLATFORM_SWITCH_ON_TIME  20

namespace OCL
{
  
class xyPlatformAxisHardware
{
public:
  xyPlatformAxisHardware(Event<void(void)>& maximumDrive, std::string configfile );
  ~xyPlatformAxisHardware();

  bool prepareForUse();
  bool prepareForShutdown();
  std::vector<ORO_DeviceInterface::AxisInterface*> xyPlatformAxisHardware::getAxes();
  void unlock(int axis);
  void lock(int axis);
  void stop(int axis);
  double getDriveValue(int axis);
  void addOffset(const std::vector<double>& offset);
  bool getReference(int axis);
  void writePosition(int axis, double q);

  RTT::TaskContext* getTaskContext();


private:
  std::vector<Axis*> _axes;
  std::vector<RTT::AxisInterface*> _axesInterface;
  
  RTT::EncoderInterface*             _encoderInterface[XY_NUM_AXIS];
  AnalogOutput<unsigned int>*      _vref[XY_NUM_AXIS];
  IncrementalEncoderSensor*        _encoder[XY_NUM_AXIS];
  DigitalOutput*                   _enable[XY_NUM_AXIS];
  AnalogDrive*                     _drive[XY_NUM_AXIS];
  DigitalOutput*                   _brake[XY_NUM_AXIS];
  //  DigitalInput*                    _reference[KUKA160_NUM_AXIS];  

  RTT::TaskContext _my_task_context;
  
  std::string _configfile;
  bool _initialized;

};


} //namespace

#endif    // XYPLATFORMCREATOR_HPP
