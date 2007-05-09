/***************************************************************************
  tag: Ruben Smits  Mon Jan 19 14:11:20 CET 2004  JR3WrenchSensor.hpp 

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

#include "xyPlatformAxisSimulation.hpp"

#define XY_JOINT_START_ANGLE  {0.0, 0.0}

using namespace ORO_DeviceDriver;
using namespace ORO_Execution;


xyPlatformAxisSimulation::xyPlatformAxisSimulation()
  :_axes(XY_NUM_AXIS),
   _axesInterface(XY_NUM_AXIS),
   _reference(XY_NUM_AXIS),
   _my_task_context("XYAxis")
{
    double jointspeedlimits[XY_NUM_AXIS] = XY_JOINTSPEEDLIMITS;
    double qinit[XY_NUM_AXIS] = XY_JOINT_START_ANGLE;
    //    double offsets[XY_NUM_AXIS] = XY_OFFSETSinVOLTS;
    //    double scales[XY_NUM_AXIS] = XY_SCALES;
  
    for (unsigned int i = 0; i <XY_NUM_AXIS; i++){
      _axes[i] = new ORO_DeviceDriver::SimulationAxis(qinit[i]);
      _axes[i]->setMaxDriveValue( jointspeedlimits[i] );
      _axesInterface[i] = _axes[i];
      _reference[i]=true;
    }
    

  // make task context
  _my_factory = newMethodFactory( this );
  _my_factory->add("lock", method( &xyPlatformAxisSimulation::lock, "lock axis", "axis", "axis to lock"));
  _my_factory->add("unlock", method( &xyPlatformAxisSimulation::unlock, "unlock axis", "axis", "axis to unlock"));
  _my_factory->add("stop", method( &xyPlatformAxisSimulation::stop, "stop axis", "axis", "axis to stop"));
  _my_factory->add("getDriveValue", method( &xyPlatformAxisSimulation::getDriveValue, "get drive value of axis", "axis", "drive value of this axis"));
  _my_factory->add("addOffset", method( &xyPlatformAxisSimulation::addOffset, "Add offset to axis", "offset", "offset to add"));
  _my_factory->add("getReference", method( &xyPlatformAxisSimulation::getReference, "Get referencesignal from axis", "axis", "axis to get reference from"));
  _my_factory->add("writePosition", method( &xyPlatformAxisSimulation::writePosition, "Write value to axis", "axis","axis to write value to","q", "joint angle in radians"));
  _my_factory->add("prepareForUse", method( &xyPlatformAxisSimulation::prepareForUse, "Prepare robot for use"));
  _my_factory->add("prepareForShutdown", method( &xyPlatformAxisSimulation::prepareForShutdown, "Prepare robot for shutdown"));
  _my_task_context.methodFactory.registerObject("this", _my_factory);
}

xyPlatformAxisSimulation::~xyPlatformAxisSimulation()
{
  for (unsigned int i = 0; i < XY_NUM_AXIS; i++)
    delete _axes[i];
}

std::vector<ORO_DeviceInterface::AxisInterface*> 
xyPlatformAxisSimulation::getAxes()
{
  return _axesInterface;
}



void
xyPlatformAxisSimulation::unlock(int axis)
{
  _axes[axis]->unlock();
}


void
xyPlatformAxisSimulation::stop(int axis)
{
  _axes[axis]->stop();
}


void
xyPlatformAxisSimulation::lock(int axis)
{
  _axes[axis]->lock();
}



double
xyPlatformAxisSimulation::getDriveValue(int axis)
{
  if (!(axis<0 || axis>XY_NUM_AXIS-1))
    return _axes[axis]->getDriveValue();
  else
    return 0.0;
}


void
xyPlatformAxisSimulation::addOffset(const std::vector<double>& offset)
{}

void
xyPlatformAxisSimulation::writePosition(int axis, double q)
{}

bool
xyPlatformAxisSimulation::getReference(int axis)
{
  assert(!(axis<0 || axis>XY_NUM_AXIS-1));
  _reference[axis]= !_reference[axis];
  return _reference[axis];
}

TaskContext*
xyPlatformAxisSimulation::getTaskContext()
{
  return &_my_task_context;
}
