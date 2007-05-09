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

#include "xyPlatformConstants.hpp"
#include "xyPlatformAxisHardware.hpp"
#include <fstream>
#include <iostream>
#include <assert.h>
#include <rtt/Method.hpp>

using namespace RTT;
using namespace OCL;
using namespace std;

namespace OCL
{
  
  class VelocityReaderXY : public SensorInterface<double>
  {
  public:
    VelocityReaderXY(Axis* axis, double maxvel) : _axis(axis), _maxvel(maxvel)
    {};
    
    virtual ~VelocityReaderXY() {};
    
    virtual int readSensor( double& vel ) const { vel = _axis->getDriveValue(); return 0; }
    
    virtual double readSensor() const { return _axis->getDriveValue(); }
    
    virtual double maxMeasurement() const { return _maxvel; }
    
    virtual double minMeasurement() const { return -_maxvel; }
    
    virtual double zeroMeasurement() const { return 0; }
    
  private:
    Axis* _axis;
    double _maxvel;
  };
  
} // namespace

xyPlatformAxisHardware::xyPlatformAxisHardware(Event<void(void)>& maximumDrive, std::string configfile )
  : _axes(XY_NUM_AXIS),
    _axesInterface(XY_NUM_AXIS),
    _my_task_context("XYAxis"),
    _configfile(configfile),
    _initialized(false)
{
    // set constants
  double mmpsec2volt[XY_NUM_AXIS] = XY_SCALES;
  double encoderOffsets[XY_NUM_AXIS] = XY_ENCODEROFFSETS;
  double ticks2mm[XY_NUM_AXIS] = XY_PULSES_PER_MM;
  double jointspeedlimits[XY_NUM_AXIS] = XY_JOINTSPEEDLIMITS;
  double driveOffsets[XY_NUM_AXIS];
  ifstream offsetfile(_configfile.c_str(), ios::in);  bool success = true;
  for (unsigned int i=0; i<XY_NUM_AXIS; i++){
    driveOffsets[i] = 0;
    if ( ! (offsetfile >> driveOffsets[i]) )  success = false;
  }
  if (!success)  cout << "(xyPlatformAxisHardware) Offset file not read. Setting offsets to zero" << endl;
    
  _comediDevAOut = new ComediDevice( 1 );
  _comediDevEncoder = new ComediDevice( 2 );
  _comediDevDInOut = new ComediDevice(3);
  
  int subd;
  subd = 1; // subdevice 1 is analog out
  _comediSubdevAOut = new ComediSubDeviceAOut( _comediDevAOut, "xyPlatform", subd);

  subd = 1; // subdevice 1 is digital out
  _comediSubdevDOut = new ComediSubDeviceDOut ( _comediDevDInOut, "xyPlatform", subd);
  
  subd = 0; // subdevice 0 is counter
  //Setting up encoders
  _encoderInterface[0] = new ComediEncoder(_comediDevEncoder , subd , 6);
  _encoderInterface[1] = new ComediEncoder(_comediDevEncoder , subd , 7);
  _encoder[0] = new IncrementalEncoderSensor( _encoderInterface[0], -1000 * ticks2mm[0], encoderOffsets[0], X_AXIS_MIN_POS, X_AXIS_MAX_POS, 4096);
  _encoder[1] = new IncrementalEncoderSensor( _encoderInterface[1], -1000 * ticks2mm[1], encoderOffsets[1], Y_AXIS_MIN_POS, Y_AXIS_MAX_POS, 4096);
  //  _encoder[0] = new IncrementalEncoderSensor( _encoderInterface[0], 400000, 0, 0, 0.5, 4096);
  //  _encoder[1] = new IncrementalEncoderSensor( _encoderInterface[1], 400000, 0, 0, 0.5, 4096);
  

  
  for (unsigned int i = 0; i < XY_NUM_AXIS; i++){
    _brake[i] = new DigitalOutput( ); // Virtual (software) signal
    // _brake[i]->switchOn();
    
    _vref[i]   = new AnalogOutput<unsigned int>( _comediSubdevAOut, i + 6 );
    _enable[i] = new DigitalOutput( );// Virtual (software) signal
    //_reference[i] = new DigitalInput( _comediSubdevDIn, 23 - i);
    _drive[i] = new AnalogDrive( _vref[i], _enable[i], 1.0 / mmpsec2volt[i], driveOffsets[i] / mmpsec2volt[i]);
    
    _axes[i] = new Axis( _drive[i] );
    _axes[i]->limitDrive( jointspeedlimits[i] );
    _axes[i]->setLimitDriveEvent( maximumDrive );
    _axes[i]->setBrake( _brake[i] );
    _axes[i]->setSensor( "Position", _encoder[i] );
    _axes[i]->setSensor( "Velocity", new VelocityReaderXY(_axes[i], jointspeedlimits[i] ) );
    
    _axesInterface[i] = _axes[i];
  }

  // make task context
    this->methods()->addMethod(method( "lock", &xyPlatformAxisHardware::lock, this), "lock axis", "axis", "axis to lock");
    this->methods()->addMethod(method( "unlock", &xyPlatformAxisHardware::unlock, this), "unlock axis", "axis", "axis to unlock");
    this->methods()->addMethod(method( "stop", &xyPlatformAxisHardware::stop, this), "stop axis", "axis", "axis to stop");
    this->methods()->addMethod(method( "getDriveValue", &xyPlatformAxisHardware::getDriveValue, this), "get drive value of axis", "axis", "drive value of this axis");
    this->methods()->addMethod(method( "addOffset", &xyPlatformAxisHardware::addOffset,this ), "Add offset to axis", "offset", "offset to add");
    this->methods()->addMethod(method( "getReference", &xyPlatformAxisHardware::getReference, this), "Get referencesignal from axis", "axis", "axis to get reference from");
    this->methods()->addMethod(method( "writePosition", &xyPlatformAxisHardware::writePosition, this), "Write value to axis", "axis","axis to write value to","q", "joint angle in radians");
    this->methods()->addMethod(method( "prepareForUse", &xyPlatformAxisHardware::prepareForUse, this), "Prepare robot for use");
    this->methods()->addMethod(method( "prepareForShutdown", &xyPlatformAxisHardware::prepareForShutdown, this), "Prepare robot for shutdown");
}

xyPlatformAxisHardware::~xyPlatformAxisHardware()
{
    // make sure robot is shut down
  prepareForShutdown();

  // write offsets to file
  ofstream offsetfile(_configfile.c_str(), ios::out);
  for (unsigned int i=0; i<XY_NUM_AXIS; i++)
    offsetfile << "  " << (_axes[i]->getDrive()->getOffset());

  for (unsigned int i = 0; i < XY_NUM_AXIS; i++){
    // brake, drive, encoders are deleted by each axis
    delete _axes[i];
  }

  delete _comediDevAOut;
  delete _comediDevEncoder;
  delete _comediSubdevAOut;
  delete _comediDevDInOut;
  delete _comediSubdevDOut;
  
}


bool xyPlatformAxisHardware::prepareForUse()
{
  if (!_initialized){
    // first switch all channels off
    for(int i = 0; i < 24 ; i++)  _comediSubdevDOut->switchOff( i );
    // channel 7: rcm-monitoring
    _comediSubdevDOut->switchOn( 7 );
    _initialized = true;
  }


  // robot was switched on
  if (_initialized)
    return true;
  else 
    return false;
}




bool xyPlatformAxisHardware::prepareForShutdown()
{
  _comediSubdevDOut->switchOff(7);
  _initialized = false;
  return true;
}


std::vector<AxisInterface*> 
xyPlatformAxisHardware::getAxes()
{
  return _axesInterface;
}



void
xyPlatformAxisHardware::unlock(int axis)
{
  if (!(axis<0 || axis>XY_NUM_AXIS-1))
    _axes[axis]->unlock();
}


void
xyPlatformAxisHardware::stop(int axis)
{
  if (!(axis<0 || axis>XY_NUM_AXIS-1))
    _axes[axis]->stop();
}


void
xyPlatformAxisHardware::lock(int axis)
{
  if (!(axis<0 || axis>XY_NUM_AXIS-1))
    _axes[axis]->lock();
}



double
xyPlatformAxisHardware::getDriveValue(int axis)
{
  if (!(axis<0 || axis>XY_NUM_AXIS-1))
    return _axes[axis]->getDriveValue();
  else
    return 0.0;
}


void
xyPlatformAxisHardware::addOffset(const std::vector<double>& offset)
{
  assert(offset.size() == XY_NUM_AXIS);
  for (unsigned int i=0; i<XY_NUM_AXIS; i++)
    _axes[i]->getDrive()->addOffset(offset[i]);
}

bool
xyPlatformAxisHardware::getReference(int axis)
{
  assert(!(axis<0 || axis>XY_NUM_AXIS-1));
  return true;
}

void
xyPlatformAxisHardware::writePosition(int axis, double q)
{
  assert(!(axis<0 || axis>XY_NUM_AXIS-1));
  _encoder[axis]->writeSensor(q);
}


TaskContext*
xyPlatformAxisHardware::getTaskContext()
{
  return &_my_task_context;
}
