/***************************************************************************
 tag: Erwin Aertbelien May 2006
                           -------------------
    begin                : Mon January 19 2004
    copyright            : (C) 2004 Peter Soetens
    email                : Erwin.Aertbelien@mech.kuleuven.ac.be
 
 based on the work of Johan Rutgeerts in LiASHardware.cpp

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

#include "LiASnAxesVelocityController.hpp"

#include <execution/GenericTaskContext.hpp>
#include <corelib/NonPreemptibleActivity.hpp>
#include <execution/TemplateFactories.hpp>
#include <execution/BufferPort.hpp>
#include <corelib/Event.hpp>
#include <corelib/Logger.hpp>
#include <corelib/Attribute.hpp>
#include <execution/DataPort.hpp>
#include <device_drivers/IncrementalEncoderSensor.hpp>
#include <device_drivers/AnalogOutput.hpp>
#include <device_drivers/DigitalOutput.hpp>
#include <device_drivers/DigitalInput.hpp>
#include <device_drivers/AnalogDrive.hpp>
#include <device_drivers/Axis.hpp>
#include <device_interface/AxisInterface.hpp>


#if defined (OROPKG_OS_LXRT)

#include "IP_Digital_24_DOutInterface.hpp"
#include "IP_Encoder_6_EncInterface.hpp"
#include "IP_FastDAC_AOutInterface.hpp"
#include "IP_OptoInput_DInInterface.hpp"
#include "CombinedDigitalOutInterface.hpp"

#include "LiASConstants.hpp"

#else

#include <device_drivers/SimulationAxis.hpp>

#endif
namespace Orocos {

	using namespace RTT;
	using namespace std;

#define NUM_AXES 6

#define DBG \
    Logger::log() << Logger::Info << Logger::endl; \
    Logger::log() << Logger::Info << __PRETTY_FUNCTION__ << Logger::endl;


#define TRACE(x) \
    Logger::log() << Logger::Info << "TRACE " << #x << Logger::endl; \
    x;
    

using namespace Orocos;


LiASnAxesVelocityController::LiASnAxesVelocityController(const std::string& name,const std::string& propertyfilename)
  : GenericTaskContext(name),
    driveValue(NUM_AXES),
    reference(NUM_AXES),
    positionValue(NUM_AXES),
    output(NUM_AXES),
    _propertyfile(propertyfilename),
    driveLimits("driveLimits","velocity limits of the axes, (rad/s)"),
    lowerPositionLimits("LowerPositionLimits","Lower position limits (rad)"),
    upperPositionLimits("UpperPositionLimits","Upper position limits (rad)"),
    initialPosition("initialPosition","Initial position (rad) for simulation or hardware"),
    signAxes("signAxes","Indicates the sign of each of the axes"),
 	offset  ("offset",  "offset to compensate for friction.  Should only partially compensate friction"),
    _num_axes(NUM_AXES),
	_homed(NUM_AXES),
    servoGain("servoGain","gain of the servoloop (no units)"),
    _servoGain(NUM_AXES),
    servoIntegrationFactor("servoIntegrationFactor","INVERSE of the integration time for the servoloop (s)"),
    _servoIntegrationFactor(NUM_AXES),
    servoFFScale(        "servoFFScale","scale factor on the feedforward signal of the servo loop "),
    _servoFFScale(NUM_AXES),
    servoDerivTime(      "servoDerivTime","Derivative time for the servo loop "),
    servoIntVel(NUM_AXES),
    servoIntError(NUM_AXES),
	previousPos(NUM_AXES),
    servoInitialized(false),
    _axes(NUM_AXES),
    _axesInterface(NUM_AXES)
{
  /**
   * Adding properties
   */
  attributes()->addProperty( &driveLimits );
  attributes()->addProperty( &lowerPositionLimits );
  attributes()->addProperty( &upperPositionLimits  );
  attributes()->addProperty( &initialPosition  );
  attributes()->addProperty( &signAxes  );
  attributes()->addProperty( &offset  );
  attributes()->addProperty( &servoGain  );
  attributes()->addProperty( &servoIntegrationFactor  );
  attributes()->addProperty( &servoFFScale  );
  attributes()->addProperty( &servoDerivTime  );
  attributes()->addConstant( "NUM_AXES", &_num_axes);
 
  if (!readProperties(propertyfilename)) {
    Logger::log() << Logger::Error << "Failed to read the property file, continueing with default values." << Logger::endl;
    throw 0;
  }
#if defined (OROPKG_OS_LXRT)
    Logger::log() << Logger::Info << "LXRT version of LiASnAxesVelocityController has started" << Logger::endl;
    
    _IP_Digital_24_DOut = new IP_Digital_24_DOutInterface("IP_Digital_24_DOut");
    // \TODO : Set this automatically to the correct value :
    _IP_Encoder_6_task  = new IP_Encoder_6_Task(LiAS_ENCODER_REFRESH_PERIOD);
    _IP_FastDac_AOut    = new IP_FastDAC_AOutInterface("IP_FastDac_AOut");
    _IP_OptoInput_DIn   = new IP_OptoInput_DInInterface("IP_OptoInput_DIn");
   
    _IP_Encoder_6_task->start();

    //Set all constants        
    double driveOffsets[6] = LiAS_OFFSETSinVOLTS;
    double radpsec2volt[6] = LiAS_RADproSEC2VOLT;
    int    encoderOffsets[6] = LiAS_ENCODEROFFSETS;
    double ticks2rad[6] = LiAS_TICKS2RAD;
    double jointspeedlimits[6] = LiAS_JOINTSPEEDLIMITS;

    
    _enable = new DigitalOutput( _IP_Digital_24_DOut, LiAS_ENABLE_CHANNEL, true);
    _enable->switchOff();
    
    _combined_enable_DOutInterface = new CombinedDigitalOutInterface( "enableDOutInterface", _enable, 6, OR );
   
    
    _brake = new DigitalOutput( _IP_Digital_24_DOut, LiAS_BRAKE_CHANNEL, false);
    _brake->switchOn();
    
    _combined_brake_DOutInterface = new CombinedDigitalOutInterface( "brakeDOutInterface", _brake, 2, OR);
   
    
    for (unsigned int i = 0; i < LiAS_NUM_AXIS; i++)
    {
		_homed[i] = false;
        //Setting up encoders
        _encoderInterface[i] = new IP_Encoder_6_EncInterface( *_IP_Encoder_6_task, i ); // Encoder 0, 1, 2, 3, 4 and 5.
        _encoder[i] = new IncrementalEncoderSensor( _encoderInterface[i], 1.0 / ticks2rad[i], encoderOffsets[i], -180, 180, LiAS_ENC_RES );
        
        _vref[i]   = new AnalogOutput<unsigned int>( _IP_FastDac_AOut, i + 1 ); 
        _combined_enable[i] = new DigitalOutput( _combined_enable_DOutInterface, i );
        _drive[i] = new AnalogDrive( _vref[i], _combined_enable[i], 1.0 / radpsec2volt[i], driveOffsets[i] / radpsec2volt[i]);
        
        _reference[i] = new DigitalInput( _IP_OptoInput_DIn, i + 1 ); // Bit 1, 2, 3, 4, 5, and 6. 
        
        
        _axes[i] = new ORO_DeviceDriver::Axis( _drive[i] );
        _axes[i]->limitDrive( jointspeedlimits[i] );
        //_axes[i]->setLimitDriveEvent( maximumDrive );  \\TODO I prefere to handle this myself.
        _axes[i]->setSensor( "Position", _encoder[i] );
        // not used any more :_axes[i]->setSensor( "Velocity", new VelocityReaderLiAS( _axes[i], jointspeedlimits[i] ) );
      
        // Axis 2 and 3 get a combined brake
        if ((i == 1)||(i == 2))
        {
            _combined_brake[i-1] = new DigitalOutput( _combined_brake_DOutInterface, i-1);
            _axes[i]->setBrake( _combined_brake[i-1] );
        }

        _axesInterface[i] = _axes[i];
    }
  #else  // ifndef   OROPKG_OS_LXRT
    Logger::log() << Logger::Info << "GNU-Linux simulation version of LiASnAxesVelocityController has started" << Logger::endl;
    /*_IP_Digital_24_DOut            = 0;
    _IP_Encoder_6_task             = 0;
    _IP_FastDac_AOut               = 0;
    _IP_OptoInput_DIn              = 0;
    _combined_enable_DOutInterface = 0;
    _enable                        = 0;
    _combined_brake_DOutInterface  = 0;
    _brake                         = 0;*/
  
    for (unsigned int i = 0; i <NUM_AXES; i++) {
	  _homed[i] = true;
      _axes[i] = new ORO_DeviceDriver::SimulationAxis(
					initialPosition.value()[i],
					lowerPositionLimits.value()[i],
					upperPositionLimits.value()[i]);
      _axes[i]->setMaxDriveValue( driveLimits.value()[i] );
      _axesInterface[i] = _axes[i];
    }
  #endif    

  _deactivate_axis3 = false;
  _deactivate_axis2 = false;
  _activate_axis2   = false;
  _activate_axis3   = false;

  /*
   *  Command Interface
   */
  typedef LiASnAxesVelocityController MyType;
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
  cfact->add( "changeServo"     ,command( &MyType::changeServo,         &MyType::changeServoCompleted,  "Apply the changed properties of the servo loop") );
  this->commands()->registerObject("this", cfact);
  
  TemplateMethodFactory<MyType>* cmeth = newMethodFactory( this );
  cmeth->add( "isDriven",         method( &MyType::isDriven,  "checks wether axis is driven","axis","axis to check" ) );


  this->methods()->registerObject("this", cmeth);


  /**
   * Creating and adding the data-ports
   */
  for (int i=0;i<NUM_AXES;++i) {
      char buf[80];
      sprintf(buf,"driveValue%d",i);
      driveValue[i] = new ReadDataPort<double>(buf);
      ports()->addPort(driveValue[i]);
      sprintf(buf,"reference%d",i);
      reference[i] = new WriteDataPort<bool>(buf);
      ports()->addPort(reference[i]);
      sprintf(buf,"positionValue%d",i);
      positionValue[i]  = new WriteDataPort<double>(buf);
      ports()->addPort(positionValue[i]);
      sprintf(buf,"output%d",i);
      output[i]  = new WriteDataPort<double>(buf);
      ports()->addPort(output[i]);
  }

  /**
   * Adding the events :
   */
   events()->addEvent( "driveOutOfRange", &driveOutOfRange );
   events()->addEvent( "positionOutOfRange", &positionOutOfRange );

   /**
	* Connecting EventC to Events making c++-emit possible
	*/
	driveOutOfRange_eventc = events()->setupEmit("driveOutOfRange").arg(driveOutOfRange_axis).arg(driveOutOfRange_value);
	positionOutOfRange_eventc = events()->setupEmit("positionOutOfRange").arg(positionOutOfRange_axis).arg(positionOutOfRange_value);

   /**
    * Initializing servoloop
    */
    for (int axis=0;axis<NUM_AXES;++axis) {
        // state      :
        servoIntVel[axis]   = initialPosition.value()[axis];
        previousPos[axis]   = initialPosition.value()[axis];
        servoIntError[axis] = 0;  // for now.  Perhaps store it and reuse it in a property file.
        // parameters :
        _servoGain[axis]              = servoGain.value()[axis];
        _servoIntegrationFactor[axis] = servoIntegrationFactor.value()[axis];
        _servoFFScale[axis]           = servoFFScale.value()[axis];
    }
}

LiASnAxesVelocityController::~LiASnAxesVelocityController()
{
   DBG;
  // make sure robot is shut down
  prepareForShutdown();

  for (unsigned int i = 0; i < LiAS_NUM_AXIS; i++)
  {
    // brake, drive, encoders are deleted by each axis
    delete _axes[i];
    #if defined (OROPKG_OS_LXRT)
        delete _reference[i];
        delete _encoderInterface[i];
    #endif
  }
    #if defined (OROPKG_OS_LXRT)
    delete _combined_enable_DOutInterface;
    delete _enable;
    delete _combined_brake_DOutInterface;
    delete _brake;
    if (_IP_Encoder_6_task!=0) _IP_Encoder_6_task->stop();
 
    delete _IP_Digital_24_DOut;
    delete _IP_Encoder_6_task;
    delete _IP_FastDac_AOut;
    delete _IP_OptoInput_DIn;
    #endif
}

bool
LiASnAxesVelocityController::isDriven(int axis)
{
  DBG;
  if (!(axis<0 || axis>NUM_AXES-1))
    return _axes[axis]->isDriven();
  else{
    Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
    return false;
  }
}

bool
LiASnAxesVelocityController::startAxis(int axis)
{
  DBG;
  if (!(axis<0 || axis>NUM_AXES-1))
    return _axes[axis]->drive(0.0);
  else{
    Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
    return false;
  }
}

bool 
LiASnAxesVelocityController::startAxisCompleted(int axis) const {
    DBG;
    return _axes[axis]->isDriven();
    return true;
}


bool
LiASnAxesVelocityController::startAllAxes()
{
  DBG;
  bool result = true;
  for (int axis=0;axis<NUM_AXES;++axis) {
    result &= _axes[axis]->drive(0.0);
  }
  return result;
}


bool LiASnAxesVelocityController::startAllAxesCompleted()const
{
  bool _return = true;
  for(unsigned int axis = 0;axis<NUM_AXES;axis++)
   _return &= _axes[axis]->isDriven();
  return _return;
}


bool
LiASnAxesVelocityController::stopAxis(int axis)
{
   DBG;
  if (!(axis<0 || axis>NUM_AXES-1))
    return _axes[axis]->stop();
  else{
    Logger::log()<<Logger::Error<<"Axis "<< axis <<" doesn't exist!!"<<Logger::endl;
    return false;
  }
}

bool 
LiASnAxesVelocityController::stopAxisCompleted(int axis) const {
    DBG;
    return _axes[axis]->isStopped();
}


bool
LiASnAxesVelocityController::stopAllAxes()
{
  DBG;
  bool result = true;
  for (int axis=0;axis<NUM_AXES;++axis) {
    result &= _axes[axis]->stop();
  }
  return result;
}

bool 
LiASnAxesVelocityController::stopAllAxesCompleted() const
{
  bool _return = true;
  for(unsigned int axis = 0;axis<NUM_AXES;++axis)
    _return &= _axes[axis]->isStopped();
  return _return;
}




bool
LiASnAxesVelocityController::unlockAxis(int axis)
{
  DBG;
  if (!(axis<0 || axis>LiAS_NUM_AXIS-1))
  {
      if (axis == 1) 
      {
          if( _activate_axis3 )
          {
              _activate_axis3 = false;
              _axes[1]->unlock();
              _axes[2]->unlock();
          }
          else _activate_axis2 = true;
          return true;
      }
      if (axis == 2) 
      {
          if( _activate_axis2 )
          {
              _activate_axis2 = false;
              _axes[1]->unlock();
              _axes[2]->unlock();
          }
          else _activate_axis3 = true;
          return true;
      }
      _axes[axis]->unlock();
      return true;
  } else{
    Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
    return false;
  }
}

bool 
LiASnAxesVelocityController::unlockAxisCompleted(int axis) const {
    DBG;
    //return !_axes[axis]->isLocked();
    return true;
}


bool
LiASnAxesVelocityController::lockAxis(int axis)
{
  DBG;
  if (!(axis<0 || axis>LiAS_NUM_AXIS-1))
  {
      if (axis == 1) 
      {
          if( _deactivate_axis3 )
          {
              _deactivate_axis3 = false;
              _axes[1]->lock();
              _axes[2]->lock();
          }
          else _deactivate_axis2 = true;
          return true;
      }
      if (axis == 2) 
      {
          if( _deactivate_axis2 )
          {
              _deactivate_axis2 = false;
              _axes[1]->lock();
              _axes[2]->lock();
          }
          else _deactivate_axis3 = true;
          return true;
      }
      _axes[axis]->lock();
      return true;
  } else {
    Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
    return false;
  }
}

bool 
LiASnAxesVelocityController::lockAxisCompleted(int axis) const {
    DBG;
    //return _axes[axis]->isLocked();
}


bool
LiASnAxesVelocityController::lockAllAxes() {
    DBG;
    bool result=true;
    for (int axis=0;axis<NUM_AXES;axis++) {
       _axes[axis]->lock();
    }
    return result; 
}

bool 
LiASnAxesVelocityController::lockAllAxesCompleted() const {
    DBG;
    return true;
}


bool
LiASnAxesVelocityController::unlockAllAxes() {
    DBG;
    for (int axis=0;axis<NUM_AXES;axis++) {
         _axes[axis]->unlock();
    }
    return true; 
}


bool 
LiASnAxesVelocityController::unlockAllAxesCompleted() const {
    DBG;
    return true;
}

bool
LiASnAxesVelocityController::addDriveOffset(int axis, double offset)
{ 
  DBG;
  if (!(axis<0 || axis>NUM_AXES-1)) {
       #if defined (OROPKG_OS_LXRT)
       _axes[axis]->getDrive()->addOffset(offset);
       #endif
       return true;
  } else {
    Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
    return false;
  }
}

bool 
LiASnAxesVelocityController::addDriveOffsetCompleted(int axis, double ) const {
    DBG;
    return true;
}


bool
LiASnAxesVelocityController::initPosition(int axis)
{
  DBG;
  if (!(axis<0 || axis>NUM_AXES-1)) {
	   _homed[axis] = true;
       #if defined (OROPKG_OS_LXRT)
       _encoder[axis]->writeSensor(initialPosition.value()[axis]);
       servoIntVel[axis] = initialPosition.value()[axis];
       previousPos[axis] = servoIntVel[axis];
       #else
       #endif
       return true;
  } else {
    Logger::log()<<Logger::Error<<"Axis "<< axis <<"doesn't exist!!"<<Logger::endl;
    return false;
  }
}

bool 
LiASnAxesVelocityController::initPositionCompleted(int) const {
    DBG;
    return true;
}


bool LiASnAxesVelocityController::changeServo() {
    for (int axis=0;axis<NUM_AXES;++axis) {
        servoIntVel[axis] *= _servoGain[axis] / servoGain.value()[axis];
        servoIntVel[axis] *= _servoIntegrationFactor[axis] / servoIntegrationFactor.value()[axis];
        _servoGain[axis]  = servoGain.value()[axis]; 
        _servoIntegrationFactor[axis] = servoIntegrationFactor.value()[axis];
    }
    return true;
}

bool LiASnAxesVelocityController::changeServoCompleted() const {
    DBG;
    return true;
}
 

/**
 *  This function contains the application's startup code.
 *  Return false to abort startup.
 **/
bool LiASnAxesVelocityController::startup() {
    DBG;
    // Initialize the servo loop
  for (int axis=0;axis<NUM_AXES;++axis) {
    servoIntVel[axis] = _axes[axis]->getSensor("Position")->readSensor();
	previousPos[axis] = servoIntVel[axis];
  }
  return true;
}
                   
/**
 * This function is periodically called.
 */
void LiASnAxesVelocityController::update() {
#if !defined (OROPKG_OS_LXRT)
	for (int axis=0;axis<NUM_AXES;axis++) {
		double measpos = signAxes.value()[axis]*_axes[axis]->getSensor("Position")->readSensor();
/*        if(( 
            (measpos < lowerPositionLimits.value()[axis]) 
          ||(measpos > upperPositionLimits.value()[axis])
          ) && _homed[axis]) {
            // emit event.
			positionOutOfRange_axis  = axis;
			positionOutOfRange_value = measpos;
			positionOutOfRange_eventc.emit();
        }*/
		double setpoint = signAxes.value()[axis]*driveValue[axis]->Get();
        _axes[axis]->drive(setpoint);
        positionValue[axis] ->Set(measpos);
		output[axis]->Set(setpoint);
        reference[axis]->Set(false);
	}
#else
    double dt;
    // Determine sampling time :
    if (servoInitialized) {
        dt              = TimeService::Instance()->secondsSince(previousTime);
        previousTime    = TimeService::Instance()->getTicks();
    } else {
        dt = 0.0; 
    }
    previousTime        = TimeService::Instance()->getTicks();
    servoInitialized    = true;

    double outputvel[NUM_AXES];

    for (int axis=0;axis<NUM_AXES;axis++) {      
        double measpos;
        double setpoint;
        // Ask the position and perform checks in joint space.
        measpos = signAxes.value()[axis]*_encoder[axis]->readSensor();
        positionValue[axis] ->Set(  measpos );
        if(( 
            (measpos < lowerPositionLimits.value()[axis]) 
          ||(measpos > upperPositionLimits.value()[axis])
          ) && _homed[axis]) {
            // emit event.
			positionOutOfRange_axis  = axis;
			positionOutOfRange_value = measpos;
			positionOutOfRange_eventc.emit();
        }
        if (_axes[axis]->isDriven()) {
            setpoint = driveValue[axis]->Get();
        	// perform control action ( dt is zero the first time !) :
        	servoIntVel[axis]      += dt*setpoint;
        	double error            = servoIntVel[axis] - measpos;
        	servoIntError[axis]    += dt*error;
            double deriv;
            if (dt < 1E-4) {
                deriv = 0.0;
            } else {
                deriv = servoDerivTime.value()[axis]*(measpos-previousPos[axis])/dt;
            }
        	outputvel[axis]         = _servoGain[axis]* 
                (error + _servoIntegrationFactor[axis]*servoIntError[axis] + deriv) 
                + _servoFFScale[axis]*setpoint;
            // check direction of motion or desired motion :
            double offsetsign;
            if (previousPos[axis] < measpos) {
                offsetsign = 1.0;
            } else if ( previousPos[axis] > measpos ) {
                offsetsign = -1.0;
            } else {
                offsetsign = outputvel[axis] < 0.0 ? -1.0 : 1.0;
            }
            outputvel[axis] += offsetsign*offset.value()[axis];
		} else {
			outputvel[axis] = 0.0;
		}
		previousPos[axis] = measpos;
    }
    for (int axis=0;axis<NUM_AXES;axis++) {
        // send the drive value to hw and performs checks
        if (outputvel[axis] < -driveLimits.value()[axis])  {
            // emit event.
			driveOutOfRange_axis  = axis;
			driveOutOfRange_value = outputvel[axis];
			//driveOutOfRange_eventc.emit();
			// saturate
            outputvel[axis] = -driveLimits.value()[axis];
        }
        if (outputvel[axis] >  driveLimits.value()[axis]) {
            // emit event.
    		driveOutOfRange_axis  = axis;
			driveOutOfRange_value = outputvel[axis];
			//driveOutOfRange_eventc.emit();
			// saturate
            outputvel[axis] = driveLimits.value()[axis];
        }
        output[axis]->Set(outputvel[axis]);
        _axes[axis]->drive(signAxes.value()[axis]*outputvel[axis]);
        // ask the reference value from the hw 
        reference[axis]->Set( _reference[axis]->isOn());
    }
	#endif
}
 
bool LiASnAxesVelocityController::prepareForUse() {
    DBG;
    return true;
}

bool
LiASnAxesVelocityController::prepareForUseCompleted() const {
    DBG;
    return true;
}


bool LiASnAxesVelocityController::prepareForShutdown() {
    DBG;
    return true;
}

bool
LiASnAxesVelocityController::prepareForShutdownCompleted() const {
    DBG;
    return true;
}


/**
 * This function is called when the task is stopped.
 */
void LiASnAxesVelocityController::shutdown() {
    DBG;
    prepareForShutdown();
    //writeProperties(_propertyfile);
}


} // end of namespace Orocos

//#endif //OROPKG_OS_LXRT
