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

#include <rtt/Logger.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/Command.hpp>


namespace Orocos{
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
          _num_axes("NUM_AXES",NUM_AXES),
          _driveOutOfRange("driveOutOfRange"),
          _positionOutOfRange("positionOutOfRange"),
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
          
          _axes_hardware[i] = new RTT::Axis( _drive[i] );
          _axes_hardware[i]->limitDrive( _driveLimits.value()[i] );
          //_axes[i]->setLimitDriveEvent( maximumDrive );
          _axes_hardware[i]->setBrake( _brake[i] );
          _axes_hardware[i]->setSensor( "Position", _encoder[i] );
          _axes_hardware[i]->setSwitch( "Reference", _reference[i]);
          
        }
        
#endif
        for (unsigned int i = 0; i <NUM_AXES; i++){
            _axes_simulation[i] = new RTT::SimulationAxis(_initialPosition.value()[i],
                                                          _lowerPositionLimits.value()[i],
                                                          _upperPositionLimits.value()[i]);
            _axes_simulation[i]->setMaxDriveValue( _driveLimits.value()[i] );
        }
    
#if (defined OROPKG_OS_LXRT && defined OROPKG_DEVICE_DRIVERS_COMEDI)
        if(!_simulation.value()){
            for (unsigned int i = 0; i <NUM_AXES; i++)
                _axes[i] = _axes_hardware[i];
            log(Info) << "LXRT version of Kuka160nAxesVelocityController has started" << endlog();
        }
        else{
            for (unsigned int i = 0; i <NUM_AXES; i++)
                _axes[i] = _axes_simulation[i];
            log(Info) << "LXRT simulation version of Kuka160nAxesVelocityController has started" << endlog();
        }
#else
        for (unsigned int i = 0; i <NUM_AXES; i++)
            _axes[i] = _axes_simulation[i];
        log(Info) << "GNULINUX simulation version of Kuka160nAxesVelocityController has started" << endlog();
#endif
        
        // make task context
        /*
         *  Command Interface
         */
        typedef Kuka160nAxesVelocityController MyType;
        
        this->commands()->addCommand( command( "startAxis", &MyType::startAxis,&MyType::startAxisCompleted, this),
                                      "start axis, initializes drive value to zero and starts updating the drive-value with the drive-port (only possible if axis is unlocked","axis","axis to start" );
        this->commands()->addCommand( command( "stopAxis", &MyType::stopAxis,&MyType::stopAxisCompleted, this),
                                      "stop axis, sets drive value to zero and disables the update of the drive-port, (only possible if axis is started","axis","axis to stop");
        this->commands()->addCommand( command( "lockAxis", &MyType::lockAxis,&MyType::lockAxisCompleted, this),
                                      "lock axis, enables the brakes (only possible if axis is stopped","axis","axis to lock" );
        this->commands()->addCommand( command( "unlockAxis", &MyType::unlockAxis,&MyType::unlockAxisCompleted, this),
                                      "unlock axis, disables the brakes and enables the drive (only possible if axis is locked","axis","axis to unlock" );
        this->commands()->addCommand( command( "startAllAxes", &MyType::startAllAxes,&MyType::startAllAxesCompleted, this), "start all axes"  );
        this->commands()->addCommand( command( "stopAllAxes", &MyType::stopAllAxes,&MyType::stopAllAxesCompleted, this), "stops all axes"  );
        this->commands()->addCommand( command( "lockAllAxes", &MyType::lockAllAxes,&MyType::lockAllAxesCompleted, this), "locks all axes"  );
        this->commands()->addCommand( command( "unlockAllAxes", &MyType::unlockAllAxes,&MyType::unlockAllAxesCompleted, this), "unlock all axes"  );
        this->commands()->addCommand( command( "prepareForUse", &MyType::prepareForUse,&MyType::prepareForUseCompleted, this), "prepares the robot for use"  );
        this->commands()->addCommand( command( "prepareForShutdown", &MyType::prepareForShutdown,&MyType::prepareForShutdownCompleted, this),
                                      "prepares the robot for shutdown"  );
        this->commands()->addCommand( command( "addDriveOffset", &MyType::addDriveOffset,&MyType::addDriveOffsetCompleted, this),
                                      "adds an offset to the drive value of axis","axis","axis to add offset to","offset","offset value in rad/s" );
        this->commands()->addCommand( command( "initPosition", &MyType::initPosition,&MyType::initPositionCompleted, this),
                                      "changes position value to the initial position","axis","axis to initialize" );

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
        events()->addEvent( &_driveOutOfRange, "Each axis that is out of range throws a seperate event.", "A", "Axis", "V", "Value" );
        events()->addEvent( &_positionOutOfRange, "Each axis that is out of range throws a seperate event.", "A", "Axis", "P", "Position"  );
    
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
                _positionOutOfRange(axis, _positionValue[axis]->Get());
            }
            
            // send the drive value to hw and performs checks
            if (_axes[axis]->isDriven()) {
                if ((_driveValue[axis]->Get() < -_driveLimits.value()[axis]) 
                    || (_driveValue[axis]->Get() >  _driveLimits.value()[axis]))
                    {
                        _driveOutOfRange(axis, _driveValue[axis]->Get() );
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
