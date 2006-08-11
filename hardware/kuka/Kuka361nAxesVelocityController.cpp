/***************************************************************************
 tag: Wim Meeussen and Johan Rutgeerts  Mon Jan 19 14:11:20 CET 2004   
       Ruben Smits Fri 12 08:31 CET 2006
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

#include "Kuka361nAxesVelocityController.hpp"

#include <rtt/Logger.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/Command.hpp>

namespace Orocos
{
  using namespace RTT;
  using namespace std;
  
#define KUKA361_NUM_AXES 6

#define KUKA361_ENCODEROFFSETS { 1000004, 1000000, 1000002, 449784, 1035056, 1230656 }

#define KUKA361_CONV1  94.14706
#define KUKA361_CONV2  -103.23529
#define KUKA361_CONV3  51.44118
#define KUKA361_CONV4  175
#define KUKA361_CONV5  150
#define KUKA361_CONV6  131.64395

#define KUKA361_ENC_RES  4096

  // Conversion from encoder ticks to radiants
#define KUKA361_TICKS2RAD { 2*M_PI / (KUKA361_CONV1 * KUKA361_ENC_RES), 2*M_PI / (KUKA361_CONV2 * KUKA361_ENC_RES), 2*M_PI / (KUKA361_CONV3 * KUKA361_ENC_RES), 2*M_PI / (KUKA361_CONV4 * KUKA361_ENC_RES), 2*M_PI / (KUKA361_CONV5 * KUKA361_ENC_RES), 2*M_PI / (KUKA361_CONV6 * KUKA361_ENC_RES)}
  
  // Conversion from angular speed to voltage
#define KUKA361_RADproSEC2VOLT { 2.5545, 2.67804024532652, 1.37350318088664, 2.34300679603342, 2.0058, 3.3786 } //18 april 2006
  
  
  Kuka361nAxesVelocityController::Kuka361nAxesVelocityController(string name,string propertyfile)
    : GenericTaskContext(name),
      _driveValue(KUKA361_NUM_AXES),
      _positionValue(KUKA361_NUM_AXES),
      _propertyfile(propertyfile),
      _driveLimits("driveLimits","velocity limits of the axes, (rad/s)",vector<double>(KUKA361_NUM_AXES,0)),
      _lowerPositionLimits("LowerPositionLimits","Lower position limits (rad)",vector<double>(KUKA361_NUM_AXES,0)),
      _upperPositionLimits("UpperPositionLimits","Upper position limits (rad)",vector<double>(KUKA361_NUM_AXES,0)),
      _initialPosition("initialPosition","Initial position (rad) for simulation or hardware",vector<double>(KUKA361_NUM_AXES,0)),
      _driveOffset("driveOffset","offset (in rad/s) to the drive value.",vector<double>(KUKA361_NUM_AXES,0)),
      _simulation("simulation","true if simulationAxes should be used",true),
      _num_axes("NUM_AXES",KUKA361_NUM_AXES),
      _driveOutOfRange("driveOutOfRange"),
      _positionOutOfRange("positionOutOfRange"),
      _activated(false),
      _positionConvertFactor(KUKA361_NUM_AXES),
      _driveConvertFactor(KUKA361_NUM_AXES),
#if (defined OROPKG_OS_LXRT && defined OROPKG_DEVICE_DRIVERS_COMEDI&& defined (OROPKG_DEVICE_DRIVERS_APCI))
      _axes_hardware(KUKA361_NUM_AXES),
      _encoderInterface(KUKA361_NUM_AXES),
      _encoder(KUKA361_NUM_AXES),
      _vref(KUKA361_NUM_AXES),
      _enable(KUKA361_NUM_AXES),
      _drive(KUKA361_NUM_AXES),
      _brake(KUKA361_NUM_AXES),
#endif
      _axes(KUKA361_NUM_AXES),
      _axes_simulation(KUKA361_NUM_AXES)
  {
    double ticks2rad[KUKA361_NUM_AXES] = KUKA361_TICKS2RAD;
    double vel2volt[KUKA361_NUM_AXES] = KUKA361_RADproSEC2VOLT;
    for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
      _positionConvertFactor[i] = ticks2rad[i];
      _driveConvertFactor[i] = vel2volt[i];
    }
    properties()->addProperty( &_driveLimits );
    properties()->addProperty( &_lowerPositionLimits );
    properties()->addProperty( &_upperPositionLimits  );
    properties()->addProperty( &_initialPosition  );
    properties()->addProperty( &_driveOffset  );
    properties()->addProperty( &_simulation  );
    attributes()->addConstant( &_num_axes);
    
    if (!readProperties(_propertyfile)) {
      log(Error) << "Failed to read the property file, continueing with default values." << endlog();
    }  
    
#if (defined OROPKG_OS_LXRT && defined OROPKG_DEVICE_DRIVERS_COMEDI&& defined (OROPKG_DEVICE_DRIVERS_APCI))
    int encoderOffsets[KUKA361_NUM_AXES] = KUKA361_ENCODEROFFSETS;

    _comediDev        = new ComediDevice( 1 );
    _comediSubdevAOut = new ComediSubDeviceAOut( _comediDev, "Kuka361" );
    _apci1710         = new EncoderSSI_apci1710_board( 0, 1 );
    _apci2200         = new RelayCardapci2200( "Kuka361" );
    _apci1032         = new SwitchDigitalInapci1032( "Kuka361" );
        
        
    for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++){
      //Setting up encoders
      _encoderInterface[i] = new EncoderSSI_apci1710( i + 1, _apci1710 );
      _encoder[i]          = new AbsoluteEncoderSensor( _encoderInterface[i], 1.0 / ticks2rad[i], encoderOffsets[i], -10, 10 );
      
      _brake[i] = new DigitalOutput( _apci2200, i + KUKA361_NUM_AXES );
      _brake[i]->switchOn();
      
      _vref[i]   = new AnalogOutput<unsigned int>( _comediSubdevAOut, i );
      _enable[i] = new DigitalOutput( _apci2200, i );
      _drive[i]  = new AnalogDrive( _vref[i], _enable[i], 1.0 / vel2volt[i], _driveOffset.value()[i]);
      
      _axes_hardware[i] = new RTT::Axis( _drive[i] );
      _axes_hardware[i]->setBrake( _brake[i] );
      _axes_hardware[i]->setSensor( "Position", _encoder[i] );

      _axes_hardware[i]->limitDrive(-_driveLimits.value()[i], _driveLimits.value()[i], _driveOutOfRange);
    }
    
#endif
    for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++){
        _axes_simulation[i] = new RTT::SimulationAxis(_initialPosition.value()[i],
                                                      _lowerPositionLimits.value()[i],
                                                      _upperPositionLimits.value()[i]);
    }
#if (defined OROPKG_OS_LXRT && defined OROPKG_DEVICE_DRIVERS_COMEDI&& defined (OROPKG_DEVICE_DRIVERS_APCI))
    if(!_simulation.value()){
      for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++)
	_axes[i] = _axes_hardware[i];
      log(Info) << "LXRT version of LiASnAxesVelocityController has started" << endlog();
    }
    else{
      for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++)
	_axes[i] = _axes_simulation[i];
      log(Info) << "LXRT simulation version of Kuka361nAxesVelocityController has started" << endlog();
    }
#else
    for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++)
      _axes[i] = _axes_simulation[i];
    log(Info) << "GNULINUX simulation version of Kuka361nAxesVelocityController has started" << endlog();
#endif
    
    // make task context
    /*
     *  Command Interface
     */
    typedef Kuka361nAxesVelocityController MyType;

    this->commands()->addCommand( command( "startAxis", &MyType::startAxis, &MyType::startAxisCompleted, this),
                                  "start axis, starts updating drive value, only possible after unlockAxis",
                                  "axis","axis to start" );
    this->commands()->addCommand( command( "stopAxis", &MyType::stopAxis, &MyType::stopAxisCompleted, this), 
                                  "stop axis, sets drive value to zero and stops update","axis","axis to stop" );
    this->commands()->addCommand( command( "lockAxis", &MyType::lockAxis, &MyType::lockAxisCompleted, this), 
                                  "lock axis, enables the brakes, only possible afte stopAxis","axis","axis to lock" );
    this->commands()->addCommand( command( "unlockAxis", &MyType::unlockAxis, &MyType::unlockAxisCompleted, this), 
                                  "unlock axis, unlocks the brakes and enables the drive","axis","axis to unlock" );
    this->commands()->addCommand( command( "startAllAxes", &MyType::startAllAxes, &MyType::startAllAxesCompleted, this), 
                                  "start all axes"  );
    this->commands()->addCommand( command( "stopAllAxes", &MyType::stopAllAxes, &MyType::stopAllAxesCompleted, this), 
                                  "stops all axes"  );
    this->commands()->addCommand( command( "lockAllAxes", &MyType::lockAllAxes, &MyType::lockAllAxesCompleted, this), 
                                  "locks all axes"  );
    this->commands()->addCommand( command( "unlockAllAxes", &MyType::unlockAllAxes, &MyType::unlockAllAxesCompleted, this), 
                                  "unlock all axes"  );
    this->commands()->addCommand( command( "prepareForUse", &MyType::prepareForUse, &MyType::prepareForUseCompleted, this), 
                                  "prepares the robot for use"  );
    this->commands()->addCommand( command( "prepareForShutdown", &MyType::prepareForShutdown,
                                           &MyType::prepareForShutdownCompleted, this), 
                                  "prepares the robot for shutdown"  );
    this->commands()->addCommand( command( "addDriveOffset", &MyType::addDriveOffset, &MyType::addDriveOffsetCompleted, this),  
                                  "adds an offset to the drive value of axis","axis",
                                  "axis to add offset to","offset","offset value in rad/s" );

    /**
     * Creating and adding the data-ports
     */
    for (int i=0;i<KUKA361_NUM_AXES;++i) {
        char buf[80];
        sprintf(buf,"driveValue%d",i);
        _driveValue[i] = new ReadDataPort<double>(buf);
        ports()->addPort(_driveValue[i]);
        sprintf(buf,"positionValue%d",i);
        _positionValue[i]  = new WriteDataPort<double>(buf);
        ports()->addPort(_positionValue[i]);
    }
    
    /**
     * Adding the events :
     */
    events()->addEvent( &_driveOutOfRange, "Velocity of an Axis is out of range","message","Information about event" );
    events()->addEvent( &_positionOutOfRange, "Position of an Axis is out of range","message","Information about event");
  }
  
  Kuka361nAxesVelocityController::~Kuka361nAxesVelocityController()
  {
    // make sure robot is shut down
    prepareForShutdown();
    
    // brake, drive, sensors and switches are deleted by each axis
    for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++)
      delete _axes_simulation[i];
    
#if (defined OROPKG_OS_LXRT && defined OROPKG_DEVICE_DRIVERS_COMEDI&& defined (OROPKG_DEVICE_DRIVERS_APCI))
    for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++)
      delete _axes_hardware[i];
    delete _comediDev;
    delete _comediSubdevAOut;
    delete _apci1710;
    delete _apci2200;
    delete _apci1032;
#endif
  }
  
  
  bool Kuka361nAxesVelocityController::startup()
  {
    return true;
  }
  
  void Kuka361nAxesVelocityController::update()
    {
      for (int axis=0;axis<KUKA361_NUM_AXES;axis++) {      
      // Set the position and perform checks in joint space.
      _positionValue[axis]->Set(_axes[axis]->getSensor("Position")->readSensor());
      
      // emit event when position is out of range
      if( (_positionValue[axis]->Get() < _lowerPositionLimits.value()[axis]) ||
          (_positionValue[axis]->Get() > _upperPositionLimits.value()[axis]) )
          _positionOutOfRange("Position  of a Kuka361 Axis is out of range");
      
      // send the drive value to hw and performs checks
      if (_axes[axis]->isDriven()) 
          _axes[axis]->drive(_driveValue[axis]->Get());
      }
  }
    
    
  void Kuka361nAxesVelocityController::shutdown()
  {
    //Make sure machine is shut down
    prepareForShutdown();
    //Write properties back to file
    writeProperties(_propertyfile);
  }
  
  
  bool Kuka361nAxesVelocityController::prepareForUse()
  {
#if (defined OROPKG_OS_LXRT&& defined OROPKG_DEVICE_DRIVERS_COMEDI&& defined (OROPKG_DEVICE_DRIVERS_APCI))
    if(!_simulation.value()){
        _apci2200->switchOn( 12 );
        _apci2200->switchOn( 14 );
        Logger::log()<<Logger::Warning<<"Release Emergency stop and push button to start ...."<<Logger::endl;
    }
#endif
    _activated = true;
    return true;
  }
  
  bool Kuka361nAxesVelocityController::prepareForUseCompleted()const
  {
#if (defined OROPKG_OS_LXRT&& defined OROPKG_DEVICE_DRIVERS_COMEDI&& defined (OROPKG_DEVICE_DRIVERS_APCI))
      if(!_simulation.rvalue())
          return (_apci1032->isOn(12) && _apci1032->isOn(14));
      else
#endif
    return true;
  }
  
  bool Kuka361nAxesVelocityController::prepareForShutdown()
  {
    //make sure all axes are stopped and locked
    stopAllAxes();
    lockAllAxes();
#if (defined OROPKG_OS_LXRT&& defined OROPKG_DEVICE_DRIVERS_COMEDI&& defined (OROPKG_DEVICE_DRIVERS_APCI))
    if(!_simulation.value()){
      _apci2200->switchOff( 12 );
      _apci2200->switchOff( 14 );
    }
    
#endif
    _activated = false;
    return true;
  }
  
  bool Kuka361nAxesVelocityController::prepareForShutdownCompleted()const
  {
    return true;
  }
  
  bool Kuka361nAxesVelocityController::stopAxisCompleted(int axis)const
  {
    return _axes[axis]->isStopped();
  }
  
  bool Kuka361nAxesVelocityController::lockAxisCompleted(int axis)const
  {
    return _axes[axis]->isLocked();
  }
  
  bool Kuka361nAxesVelocityController::startAxisCompleted(int axis)const
  {
    return _axes[axis]->isDriven();
  }
  
  bool Kuka361nAxesVelocityController::unlockAxisCompleted(int axis)const
  {
    return !_axes[axis]->isLocked();
  }
  
  bool Kuka361nAxesVelocityController::stopAxis(int axis)
  {
      if (!(axis<0 || axis>KUKA361_NUM_AXES-1))
          return _axes[axis]->stop();
      else{
          Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
          return false;
      }
  }
  
  bool Kuka361nAxesVelocityController::startAxis(int axis)
  {
    if (!(axis<0 || axis>KUKA361_NUM_AXES-1))
        return _axes[axis]->drive(0.0);
    else{
      Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
      return false;
    }
  }
  
  bool Kuka361nAxesVelocityController::unlockAxis(int axis)
  {
    if(_activated){
      if (!(axis<0 || axis>KUKA361_NUM_AXES-1))
        return _axes[axis]->unlock();
      else{
        Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
        return false;
      }
    }
    else
      return false;
  }
  
  bool Kuka361nAxesVelocityController::lockAxis(int axis)
  {
    if (!(axis<0 || axis>KUKA361_NUM_AXES-1))
      return _axes[axis]->lock();
    else{
      Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
      return false;
    }
  }
  
  bool Kuka361nAxesVelocityController::stopAllAxes()
  {
    bool _return = true;
    for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
      _return &= stopAxis(i);
    }
    return _return;
  }
  
  bool Kuka361nAxesVelocityController::startAllAxes()
  {
    bool _return = true;
    for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
      _return &= startAxis(i);
    }
    return _return;
  }
  
  bool Kuka361nAxesVelocityController::unlockAllAxes()
  {
    bool _return = true;
    for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
      _return &= unlockAxis(i);
      }
      return _return;
  }
  
  bool Kuka361nAxesVelocityController::lockAllAxes()
  {
    bool _return = true;
    for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
      _return &= lockAxis(i);
    }
    return _return;
  }
  
  bool Kuka361nAxesVelocityController::stopAllAxesCompleted()const
  {
    bool _return = true;
    for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++)
      _return &= stopAxisCompleted(i);
    return _return;
  }
  
  bool Kuka361nAxesVelocityController::startAllAxesCompleted()const
  {
    bool _return = true;
    for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++)
     _return &= startAxisCompleted(i);
    return _return;
  }
  
  bool Kuka361nAxesVelocityController::lockAllAxesCompleted()const
  {
    bool _return = true;
    for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++)
      _return &= lockAxisCompleted(i);
    return _return;
  }
  
  bool Kuka361nAxesVelocityController::unlockAllAxesCompleted()const
  {
    bool _return = true;
    for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++)
      _return &= unlockAxisCompleted(i);
    return _return;
  }
  
  bool Kuka361nAxesVelocityController::addDriveOffset(int axis, double offset)
  {
    _driveOffset.value()[axis] += offset;

#if (defined OROPKG_OS_LXRT&& defined OROPKG_DEVICE_DRIVERS_COMEDI&& defined (OROPKG_DEVICE_DRIVERS_APCI))
    if (!_simulation.value())
        ((Axis*)(_axes[axis]))->getDrive()->addOffset(offset);
#endif
    return true;
  }
  
  bool Kuka361nAxesVelocityController::addDriveOffsetCompleted(int axis)const
  {
    return true;
  }
  
}//namespace orocos
