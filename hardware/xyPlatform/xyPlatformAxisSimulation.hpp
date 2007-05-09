/***************************************************************************
  tag: Ruben Smits Mon Jan 19 14:11:20 CET 2004  JR3WrenchSensor.hpp 

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

#ifndef __XYPLATFORM_AXIS_SIMULATION__
#define __XYPLATFORM_AXIS_SIMULATION__

#include "xyPlatformConstants.hpp"
#include <device_drivers/SimulationAxis.hpp>
#include <device_interface/AxisInterface.hpp>
#include <execution/TemplateFactories.hpp>
#include <execution/TaskContext.hpp>

namespace ORO_DeviceDriver
{
  
class xyPlatformAxisSimulation
{
public:
  xyPlatformAxisSimulation();
  ~xyPlatformAxisSimulation();
  
  std::vector<ORO_DeviceInterface::AxisInterface*> getAxes();
  void unlock(int axis);
  void lock(int axis);
  void stop(int axis);
  double getDriveValue(int axis);
  void addOffset(const std::vector<double>& offset);
  bool getReference(int axis);
  void writePosition(int axis, double q);
  bool prepareForUse(){return true;};
  bool prepareForShutdown(){return true;};
  ORO_Execution::TaskContext* getTaskContext();

private:
  std::vector<ORO_DeviceDriver::SimulationAxis*>   _axes;
  std::vector<ORO_DeviceInterface::AxisInterface*> _axesInterface;
  std::vector<bool>                                _reference;

  ORO_Execution::TaskContext _my_task_context;
  ORO_Execution::TemplateMethodFactory<xyPlatformAxisSimulation>* _my_factory;
};
 
} // namespace

#endif    // XYPLATFORM_AXIS_SIMULATION_HPP
