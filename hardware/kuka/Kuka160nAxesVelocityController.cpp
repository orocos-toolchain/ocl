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

#include "Kuka160nAxesVelocityController.hpp"

#include <execution/TemplateFactories.hpp>
#include <corelib/Logger.hpp>
#include <corelib/Attribute.hpp>

namespace Orocos
{
  using namespace RTT;
  using namespace std;
  
#define NUM_AXES 6
#define KUKA160_CONV1  120*114*106*100/( 30*40*48*14)
#define KUKA160_CONV2  168*139*111/(28*37*15)
#define KUKA160_CONV3  168*125*106/(28*41*15)
#define KUKA160_CONV4  150.857
#define KUKA160_CONV5  155.17
#define KUKA160_CONV6  100
  
  // Resolution of the encoders
#define KUKA160_ENC_RES  4096
  
  // Conversion from encoder ticks to radiants
#define KUKA160_TICKS2RAD { 2*M_PI / (KUKA160_CONV1 * KUKA160_ENC_RES), 2*M_PI / (KUKA160_CONV2 * KUKA160_ENC_RES), 2*M_PI / (KUKA160_CONV3 * KUKA160_ENC_RES), 2*M_PI / (KUKA160_CONV4 * KUKA160_ENC_RES), 2*M_PI / (KUKA160_CONV5 * KUKA160_ENC_RES), 2*M_PI / (KUKA160_CONV6 * KUKA160_ENC_RES)}
  
  // Conversion from angular speed to voltage
#define KUKA160_RADproSEC2VOLT { 3.97143, 4.40112, 3.65062, 3.38542, 4.30991, 2.75810 }
  
  
  Kuka160nAxesVelocityController::Kuka160nAxesVelocityController(string name,string propertyfile)
    : GenericTaskContext(name),
      _driveValue(NUM_AXES),
      _references(NUM_AXES),
      _positionValue(NUM_AXES),
      _propertyfile(propertyfile),
      _driveLimits("driveLimits","velocity limits of the axes, (rad/s)"),
      _lowerPositionLimits("LowerPositionLimits","Lower position limits (rad)"),
      _upperPositionLimits("UpperPositionLimits","Upper position limits (rad)"),
      _initialPosition("initialPosition","Initial position (rad) for simulation or hardware"),
      _driveOffset("driveOffset","offset (in rad/s) to the drive value."),
      _simulation("simulation","true if simulationAxes should be used"),
      _activated(false),
      _positionConvertFactor(NUM_AXES),
      _driveConvertFactor(NUM_AXES),
#if (defined OROPKG_OS_LXRT && defined OROPKG_DEVICE_DRIVERS_COMEDI)
      _axes_hardware(NUM_AXES),
#endif
      _axes(NUM_AXES),
      _axes_simulation(NUM_AXES)
  {
    double ticks2rad[NUM_AXES] = KUKA160_TICKS2RAD;
    double vel2volt[NUM_AXES] = KUKA160_RADproSEC2VOLT;
    for(unsigned int i = 0;i<NUM_AXES;i++){
      _positionConvertFactor[i] = ticks2rad[i];
      _driveConvertFactor[i] = vel2volt[i];
    }
    
    attributes()->addProperty( &_driveLimits );
    attributes()->addProperty( &_lowerPositionLimits );
    attributes()->addProperty( &_upperPositionLimits  );
    attributes()->addProperty( &_initialPosition  );
    attributes()->addProperty( &_driveOffset  );
    attributes()->addProperty( &_simulation  );
    attributes()->addConstant( "positionConvertFactor",_positionConvertFactor  );
    attributes()->addConstant( "driveConvertFactor",_driveConvertFactor  );
    attributes()->addConstant( "NUM_AXES", NUM_AXES);
    
    if (!readProperties(_propertyfile)) {
      Logger::log() << Logger::Error << "Failed to read the property file, continueing with default values." << Logger::endl;
    }  
    
#if (defined OROPKG_OS_LXRT && defined OROPKG_DEVICE_DRIVERS_COMEDI)
    _comediDevAOut       = new ComediDevice( 0 );
    _comediDevDInOut     = new ComediDevice( 3 );
    _comediDevEncoder    = new ComediDevice( 2 );
    
    int subd;
    subd = 1; // subdevice 1 is analog out
    _comediSubdevAOut    = new ComediSubDeviceAOut( _comediDevAOut, "Kuka160", subd );
    subd = 0; // subdevice 0 is digital in
    _comediSubdevDIn     = new ComediSubDeviceDIn( _comediDevDInOut, "Kuka160", subd );
    subd = 1; // subdevice 1 is digital out
    _comediSubdevDOut    = new ComediSubDeviceDOut( _comediDevDInOut, "Kuka160", subd );
  
    // first switch all channels off
    for(int i = 0; i < 24 ; i++)  _comediSubdevDOut->switchOff( i );
    
    for (unsigned int i = 0; i < NUM_AXES; i++){
      //Setting up encoders
      subd = 0; // subdevice 0 is counter
      _encoderInterface[i] = new ComediEncoder(_comediDevEncoder , subd , i);
      _encoder[i] = new IncrementalEncoderSensor( _encoderInterface[i], 1.0 / _positionConvertFactor[i], 
						  _initialPosition.value()[i]*_positionConvertFactor[i], 
						  _lowerPositionLimits.value()[i], _upperPositionLimits.value()[i],KUKA160_ENC_RES);
      _brake[i] = new DigitalOutput( _comediSubdevDOut, 23 - i,true);
      _brake[i]->switchOn();
      
      _vref[i]   = new AnalogOutput<unsigned int>( _comediSubdevAOut, i + 1 );
      _enable[i] = new DigitalOutput( _comediSubdevDOut, 13 - i );
      _reference[i] = new DigitalInput( _comediSubdevDIn, 23 - i);
      _drive[i] = new AnalogDrive( _vref[i], _enable[i], 1.0 / _driveConvertFactor[i], 
  				 _driveOffset.value()[i]);
      
      _axes_hardware[i] = new ORO_DeviceDriver::Axis( _drive[i] );
      _axes_hardware[i]->limitDrive( _driveLimits.value()[i] );
      //_axes[i]->setLimitDriveEvent( maximumDrive );
      _axes_hardware[i]->setBrake( _brake[i] );
      _axes_hardware[i]->setSensor( "Position", _encoder[i] );
      _axes_hardware[i]->setSwitch( "Reference", _reference[i]);
      
    }
    
#endif
    for (unsigned int i = 0; i <NUM_AXES; i++)
      {
  	_axes_simulation[i] = new ORO_DeviceDriver::SimulationAxis(_initialPosition.value()[i],_lowerPositionLimits.value()[i],_upperPositionLimits.value()[i]);
  	_axes_simulation[i]->setMaxDriveValue( _driveLimits.value()[i] );
      }

#if (defined OROPKG_OS_LXRT && defined OROPKG_DEVICE_DRIVERS_COMEDI)
    if(!_simulation.value()){
      for (unsigned int i = 0; i <NUM_AXES; i++)
	_axes[i] = _axes_hardware[i];
      Logger::log() << Logger::Info << "LXRT version of LiASnAxesVelocityController has started" << Logger::endl;
    }
    else{
      for (unsigned int i = 0; i <NUM_AXES; i++)
	_axes[i] = _axes_simulation[i];
      Logger::log() << Logger::Info << "LXRT simulation version of Kuka160nAxesVelocityController has started" << Logger::endl;
    }
#else
    for (unsigned int i = 0; i <NUM_AXES; i++)
      _axes[i] = _axes_simulation[i];
    Logger::log() << Logger::Info << "GNULINUX simulation version of Kuka160nAxesVelocityController has started" << Logger::endl;
#endif
    
    // make task context
    /*
     *  Command Interface
     */
    typedef Kuka160nAxesVelocityController MyType;
    TemplateCommandFactory<MyType>* cfact = newCommandFactory( this );
    cfact->add( "startAxis",         command( &MyType::startAxis,         &MyType::startAxisCompleted, "start axis, initializes drive value to zero and starts updating the drive-value with the drive-port (only possible if axis is unlocked)","axis","axis to start" ) );
    cfact->add( "stopAxis",          command( &MyType::stopAxis,          &MyType::stopAxisCompleted, "stop axis, sets drive value to zero and disables the update of the drive-port, (only possible if axis is started)","axis","axis to stop" ) );
    cfact->add( "lockAxis",          command( &MyType::lockAxis,          &MyType::lockAxisCompleted, "lock axis, enables the brakes (only possible if axis is stopped)","axis","axis to lock" ) );
    cfact->add( "unlockAxis",        command( &MyType::unlockAxis,        &MyType::unlockAxisCompleted, "unlock axis, disables the brakes and enables the drive (only possible if axis is locked)","axis","axis to unlock" ) );
    cfact->add( "startAllAxes",      command( &MyType::startAllAxes,      &MyType::startAllAxesCompleted, "start all axes" ) );
    cfact->add( "stopAllAxes",       command( &MyType::stopAllAxes,       &MyType::stopAllAxesCompleted, "stops all axes" ) );
    cfact->add( "lockAllAxes",       command( &MyType::lockAllAxes,       &MyType::lockAllAxesCompleted, "locks all axes" ) );
    cfact->add( "unlockAllAxes",     command( &MyType::unlockAllAxes,     &MyType::unlockAllAxesCompleted, "unlock all axes" ) );
    cfact->add( "prepareForUse",     command( &MyType::prepareForUse,     &MyType::prepareForUseCompleted, "prepares the robot for use" ) );
    cfact->add( "prepareForShutdown",command( &MyType::prepareForShutdown,&MyType::prepareForShutdownCompleted, "prepares the robot for shutdown" ) );
    cfact->add( "addDriveOffset"    ,command( &MyType::addDriveOffset,    &MyType::addDriveOffsetCompleted,  "adds an offset to the drive value of axis","axis","axis to add offset to","offset","offset value in rad/s") );
    cfact->add( "initPosition"     , command( &MyType::initPosition,      &MyType::initPositionCompleted,  "changes position value to the initial position","axis","axis to initialize") );
    this->commands()->registerObject("this", cfact);
    
    /**
     * Creating and adding the data-ports
     */
    for (int i=0;i<NUM_AXES;++i) {
        char buf[80];
        sprintf(buf,"driveValue%d",i);
        _driveValue[i] = new ReadDataPort<double>(buf);
        ports()->addPort(_driveValue[i]);
        sprintf(buf,"reference%d",i);
        _references[i] = new WriteDataPort<bool>(buf);
        ports()->addPort(_references[i]);
        sprintf(buf,"positionValue%d",i);
        _positionValue[i]  = new WriteDataPort<double>(buf);
        ports()->addPort(_positionValue[i]);
    }
    
    /**
     * Adding the events :
     */
    events()->addEvent( "driveOutOfRange", &_driveOutOfRange );
    events()->addEvent( "positionOutOfRange", &_positionOutOfRange );
    
    /**
     * Connecting EventC to Event make c++-emit possible
     */
    _driveOutOfRange_event = events()->setupEmit("driveOutOfRange").arg(_driveOutOfRange_axis).arg(_driveOutOfRange_value);
    _positionOutOfRange_event = events()->setupEmit("positionOutOfRange").arg(_positionOutOfRange_axis).arg(_positionOutOfRange_value);
  }
  
  Kuka160nAxesVelocityController::~Kuka160nAxesVelocityController()
  {
    // make sure robot is shut down
    prepareForShutdown();
    
    // brake, drive, sensors and switches are deleted by each axis
    for (unsigned int i = 0; i < NUM_AXES; i++)
      delete _axes_simulation[i];
    
#if (defined OROPKG_OS_LXRT && defined OROPKG_DEVICE_DRIVERS_COMEDI)
    for (unsigned int i = 0; i < NUM_AXES; i++)
      delete _axes_hardware[i];
    delete _comediDevAOut;
    delete _comediDevDInOut;
    delete _comediDevEncoder;
    delete _comediSubdevAOut;
    delete _comediSubdevDIn;
    delete _comediSubdevDOut;
#endif
  }
  
  
  bool Kuka160nAxesVelocityController::startup()
  {
    return true;
  }
  
  void Kuka160nAxesVelocityController::update()
  {
    for (int axis=0;axis<NUM_AXES;axis++) {      
      // Ask the position and perform checks in joint space.
      _positionValue[axis]->Set(_axes[axis]->getSensor("Position")->readSensor());
      
      if((_positionValue[axis]->Get() < _lowerPositionLimits.value()[axis]) 
         ||(_positionValue[axis]->Get() > _upperPositionLimits.value()[axis])
         ) {
        _positionOutOfRange_axis = axis;
	_positionOutOfRange_value = _positionValue[axis]->Get();
	_positionOutOfRange_event.emit();
      }
      
      // send the drive value to hw and performs checks
      if (_axes[axis]->isDriven()) {
        if ((_driveValue[axis]->Get() < -_driveLimits.value()[axis]) 
  	  || (_driveValue[axis]->Get() >  _driveLimits.value()[axis]))
  	{
	  _driveOutOfRange_axis = axis;
	  _driveOutOfRange_value = _driveValue[axis]->Get();
	  _driveOutOfRange_event.emit();
  	}
        else{
  	_axes[axis]->drive(_driveValue[axis]->Get());
        }
      }
      
      // ask the reference value from the hw 
#if (defined OROPKG_OS_LXRT&& defined OROPKG_DEVICE_DRIVERS_COMEDI)
      if(!_simulation.value())
	_references[axis]->Set( _axes[axis]->getSwitch("Reference")->isOn());
      else
#endif
        _references[axis]->Set(false);
    }
  }
  
  
  void Kuka160nAxesVelocityController::shutdown()
  {
    //Make sure machine is shut down
    prepareForShutdown();
    //Write properties back to file
#if (defined OROPKG_OS_LXRT&& defined OROPKG_DEVICE_DRIVERS_COMEDI)
    if(!_simulation.value())
      for(unsigned int i = 0;i<NUM_AXES;i++)    
	_driveOffset.set()[i] = ((Axis*)_axes[i])->getDrive()->getOffset();  
#endif
    writeProperties(_propertyfile);
  }
  
  
  bool Kuka160nAxesVelocityController::prepareForUse()
  {
#if (defined OROPKG_OS_LXRT&& defined OROPKG_DEVICE_DRIVERS_COMEDI)
    if(!_simulation.value()){
      _comediSubdevDOut->switchOn( 17 );
      Logger::log()<<Logger::Warning<<"Release Emergency stop and push button to start ...."<<Logger::endl;
    }
#endif
    _activated = true;
    return true;
  }
  
  bool Kuka160nAxesVelocityController::prepareForUseCompleted()const
  {
#if (defined OROPKG_OS_LXRT&& defined OROPKG_DEVICE_DRIVERS_COMEDI)
    if(!_simulation.rvalue())
      return (_comediSubdevDIn->isOn(3) && _comediSubdevDIn->isOn(5));
#endif
    return true;
  }
  
  bool Kuka160nAxesVelocityController::prepareForShutdown()
  {
    //make sure all axes are stopped and locked
    stopAllAxes();
    lockAllAxes();
#if (defined OROPKG_OS_LXRT&& defined OROPKG_DEVICE_DRIVERS_COMEDI)
    if(!_simulation.value())
      _comediSubdevDOut->switchOff( 17 );
#endif
    _activated = false;
    return true;
  }
  
  bool Kuka160nAxesVelocityController::prepareForShutdownCompleted()const
  {
    return true;
  }
  
  bool Kuka160nAxesVelocityController::stopAxisCompleted(int axis)const
  {
    return _axes[axis]->isStopped();
  }
  
  bool Kuka160nAxesVelocityController::lockAxisCompleted(int axis)const
  {
    return _axes[axis]->isLocked();
  }
  
  bool Kuka160nAxesVelocityController::startAxisCompleted(int axis)const
  {
    return _axes[axis]->isDriven();
  }
  
  bool Kuka160nAxesVelocityController::unlockAxisCompleted(int axis)const
  {
    return !_axes[axis]->isLocked();
  }
  
  bool Kuka160nAxesVelocityController::stopAxis(int axis)
  {
    if (!(axis<0 || axis>NUM_AXES-1))
      return _axes[axis]->stop();
    else{
      Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
      return false;
    }
  }
  
  bool Kuka160nAxesVelocityController::startAxis(int axis)
  {
    if (!(axis<0 || axis>NUM_AXES-1))
      return _axes[axis]->drive(0.0);
    else{
      Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
      return false;
    }
  }
  
  bool Kuka160nAxesVelocityController::unlockAxis(int axis)
  {
    if(_activated){
      if (!(axis<0 || axis>NUM_AXES-1))
        return _axes[axis]->unlock();
      else{
        Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
        return false;
      }
    }
    else
      return false;
  }
  
  bool Kuka160nAxesVelocityController::lockAxis(int axis)
  {
    if (!(axis<0 || axis>NUM_AXES-1))
      return _axes[axis]->lock();
    else{
      Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
      return false;
    }
  }
  
  bool Kuka160nAxesVelocityController::stopAllAxes()
  {
    bool _return = true;
    for(unsigned int i = 0;i<NUM_AXES;i++){
      _return &= stopAxis(i);
    }
    return _return;
  }
  
  bool Kuka160nAxesVelocityController::startAllAxes()
  {
    bool _return = true;
    for(unsigned int i = 0;i<NUM_AXES;i++){
      _return &= startAxis(i);
    }
    return _return;
  }
  
  bool Kuka160nAxesVelocityController::unlockAllAxes()
  {
    bool _return = true;
    for(unsigned int i = 0;i<NUM_AXES;i++){
      _return &= unlockAxis(i);
      }
      return _return;
  }
  
  bool Kuka160nAxesVelocityController::lockAllAxes()
  {
    bool _return = true;
    for(unsigned int i = 0;i<NUM_AXES;i++){
      _return &= lockAxis(i);
    }
    return _return;
  }
  
  bool Kuka160nAxesVelocityController::stopAllAxesCompleted()const
  {
    bool _return = true;
    for(unsigned int i = 0;i<NUM_AXES;i++)
      _return &= stopAxisCompleted(i);
    return _return;
  }
  
  bool Kuka160nAxesVelocityController::startAllAxesCompleted()const
  {
    bool _return = true;
    for(unsigned int i = 0;i<NUM_AXES;i++)
     _return &= startAxisCompleted(i);
    return _return;
  }
  
  bool Kuka160nAxesVelocityController::lockAllAxesCompleted()const
  {
    bool _return = true;
    for(unsigned int i = 0;i<NUM_AXES;i++)
      _return &= lockAxisCompleted(i);
    return _return;
  }
  
  bool Kuka160nAxesVelocityController::unlockAllAxesCompleted()const
  {
    bool _return = true;
    for(unsigned int i = 0;i<NUM_AXES;i++)
      _return &= unlockAxisCompleted(i);
    return _return;
  }
  
  bool Kuka160nAxesVelocityController::addDriveOffset(int axis, double offset)
  {
#if (defined OROPKG_OS_LXRT&& defined OROPKG_DEVICE_DRIVERS_COMEDI)
    if(!_simulation.value())
      ((Axis*)_axes[axis])->getDrive()->addOffset(offset);  
#endif
    return true;
  }
  
  bool Kuka160nAxesVelocityController::addDriveOffsetCompleted(int axis)const
  {
    return true;
  }
  
  bool Kuka160nAxesVelocityController::initPosition(int axis)
  {
#if (defined OROPKG_OS_LXRT&& defined OROPKG_DEVICE_DRIVERS_COMEDI)
    if(!_simulation.value())
      ((IncrementalEncoderSensor*)_axes[axis]->getSensor("Position"))->writeSensor(_initialPosition.value()[axis]);
#endif
    return true;
  }
  
  bool Kuka160nAxesVelocityController::initPositionCompleted(int axis)const
  {
    return true;
  }
}//namespace orocos
