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

#include "Kuka361nAxesTorqueController.hpp"

#include <rtt/Logger.hpp>

namespace OCL
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

	// Conversion factors for tacho, unknown for last 3 axes
//#define KUKA361_TACHOSCALE { 1/9.2750, 1/10.0285, 1/4.9633, 0.113, 0, 0 } 
//#define KUKA361_TACHOOFFSET { 0.0112, 0.0083, 0.0056, 0, 0, 0 } 
#define KUKA361_TACHOSCALE { 0, 0, 0, 0, 0, 0 } 
#define KUKA361_TACHOOFFSET { 0, 0, 0, 0, 0, 0 } 

	// Conversion of current to torque: Km. Initial values
    //#define KUKA361_KM { 0.2781*5.77*94.14706 / 10, 0.2863*5.85*103.23529 / 10, 0.2887*5.78*51.44118 / 10, 0.07*5.7*175 / 10, 0.07*5.7*150 / 10, 0.07*5.7*131.64395 / 10 } 
#define KUKA361_KM { 27 , 0.2863*5.85*103.23529 / 10, 0.2887*5.78*51.44118 / 10, 0.07*5.7*175 / 10, 0.07*5.7*150 / 10, 0.07*5.7*131.64395 / 10 } 

	// parameters of current regulator: I = (a*UN + b)/R + c, unknown for last 3 axes
#define KUKA361_A { 0.9030, 0.9091, 0.8929, 0.8909, 0.8962, 0.4716 }
#define KUKA361_B { 0.0896, 0.1072, 0.0867, 0.117, 0.0822, 0.0472 }
#define KUKA361_R { 0.1756, 0.1742, 0.1745, 0.1753, 0.1792, 0.1785 }
#define KUKA361_C { 0.7109, 0.3251, 0.0566, 0.1016, 0.0950, -1.0518 }
//#define KUKA361_R { 1,1,1,1,1,1 }

  // Channel position offset on DAQ-boards
#define TACHO_OFFSET 0 
#define CURRENT_OFFSET KUKA361_NUM_AXES 
#define	MODE_OFFSET 0 


    typedef Kuka361nAxesTorqueController MyType;
    
    Kuka361nAxesTorqueController::Kuka361nAxesTorqueController(string name,string propertyfile)
        : TaskContext(name),
          _startAxis( "startAxis", &MyType::startAxis, this),
          _startAllAxes( "startAllAxes", &MyType::startAllAxes, this),
          _stopAxis( "stopAxis", &MyType::stopAxis, this),
          _stopAllAxes( "stopAllAxes", &MyType::stopAllAxes, this),
          _unlockAxis( "unlockAxis", &MyType::unlockAxis, this),
          _unlockAllAxes( "unlockAllAxes", &MyType::unlockAllAxes, this),
          _lockAxis( "lockAxis", &MyType::lockAxis, this),
          _lockAllAxes( "lockAllAxes", &MyType::lockAllAxes, this),
          _prepareForUse( "prepareForUse", &MyType::prepareForUse,&MyType::prepareForUseCompleted, this),
          _prepareForShutdown( "prepareForShutdown", &MyType::prepareForShutdown,&MyType::prepareForShutdownCompleted, this),
          _addDriveOffset( "addDriveOffset", &MyType::addDriveOffset, this),
          _driveValue(KUKA361_NUM_AXES),
          _positionValue(KUKA361_NUM_AXES),
          _velocityValue(KUKA361_NUM_AXES), 
          _currentValue(KUKA361_NUM_AXES), 
          _velocityLimits("velocityLimits","velocity limits of the axes, (rad/s)",vector<double>(KUKA361_NUM_AXES,0)),
          _currentLimits("currentLimits","current limits of the axes, (A)",vector<double>(KUKA361_NUM_AXES,0)), 
          _mode("mode","control mode of the axis (velocity/torque)",vector<double>(KUKA361_NUM_AXES,0)), 
          _lowerPositionLimits("LowerPositionLimits","Lower position limits (rad)",vector<double>(KUKA361_NUM_AXES,0)),
          _upperPositionLimits("UpperPositionLimits","Upper position limits (rad)",vector<double>(KUKA361_NUM_AXES,0)),
          _initialPosition("initialPosition","Initial position (rad) for simulation or hardware",vector<double>(KUKA361_NUM_AXES,0)),
          _velDriveOffset("velDriveOffset","offset (in rad/s) to the drive value.",vector<double>(KUKA361_NUM_AXES,0)),
          _simulation("simulation","true if simulationAxes should be used",true),
          _num_axes("NUM_AXES",KUKA361_NUM_AXES),
          _velocityOutOfRange("velocityOutOfRange"),
          _currentOutOfRange("currentOutOfRange"), 
          _positionOutOfRange("positionOutOfRange"),
          _propertyfile(propertyfile),
          _activated(false),
          _positionConvertFactor(KUKA361_NUM_AXES),
          _driveConvertFactor(KUKA361_NUM_AXES),
          _tachoConvertScale(KUKA361_NUM_AXES), 
          _tachoConvertOffset(KUKA361_NUM_AXES), 
          _curReg_a(KUKA361_NUM_AXES), 
          _curReg_b(KUKA361_NUM_AXES), 
          _shunt_R(KUKA361_NUM_AXES), 
          _shunt_c(KUKA361_NUM_AXES), 
          _Km(KUKA361_NUM_AXES), 
#if (defined (OROPKG_OS_LXRT))
          _axes_hardware(KUKA361_NUM_AXES),
          _encoderInterface(KUKA361_NUM_AXES),
          _encoder(KUKA361_NUM_AXES),
          _ref(KUKA361_NUM_AXES),
          _enable(KUKA361_NUM_AXES),
          _drive(KUKA361_NUM_AXES),
          _brake(KUKA361_NUM_AXES),
          _tachoInput(KUKA361_NUM_AXES), 
          _tachometer(KUKA361_NUM_AXES), 
          _currentInput(KUKA361_NUM_AXES), 
          _currentSensor(KUKA361_NUM_AXES), 
          _modeSwitch(KUKA361_NUM_AXES), 
          //_modeCheck(KUKA361_NUM_AXES), 
#endif
          _axes(KUKA361_NUM_AXES),
          _axes_simulation(KUKA361_NUM_AXES),
          _tau_sim(KUKA361_NUM_AXES,0.0),
	  _pos_sim(KUKA361_NUM_AXES),
          _previous_time(KUKA361_NUM_AXES)
    {
        double ticks2rad[KUKA361_NUM_AXES] = KUKA361_TICKS2RAD;
        double vel2volt[KUKA361_NUM_AXES] = KUKA361_RADproSEC2VOLT;
        double tachoscale[KUKA361_NUM_AXES] = KUKA361_TACHOSCALE;
        double tachooffset[KUKA361_NUM_AXES] = KUKA361_TACHOOFFSET;
        double curReg_a[KUKA361_NUM_AXES] = KUKA361_A;
        double curReg_b[KUKA361_NUM_AXES] = KUKA361_B;
        double shunt_R[KUKA361_NUM_AXES] = KUKA361_R;
        double shunt_c[KUKA361_NUM_AXES] = KUKA361_C;
        double KM[KUKA361_NUM_AXES] = KUKA361_KM;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            _positionConvertFactor[i] = ticks2rad[i];
            _driveConvertFactor[i] = vel2volt[i];
            _tachoConvertScale[i] = tachoscale[i];
            _tachoConvertOffset[i] = tachooffset[i];
            _curReg_a[i] = curReg_a[i];
            _curReg_b[i] = curReg_b[i];
            _shunt_R[i] = shunt_R[i];
            _shunt_c[i] = shunt_c[i];
            _Km[i] = KM[i];
        }
		
        properties()->addProperty( &_velocityLimits );
        properties()->addProperty( &_currentLimits );
        properties()->addProperty( &_mode );
        properties()->addProperty( &_lowerPositionLimits );
        properties()->addProperty( &_upperPositionLimits  );
        properties()->addProperty( &_initialPosition  );
        properties()->addProperty( &_velDriveOffset  );
        properties()->addProperty( &_simulation  );
        attributes()->addConstant( &_num_axes);
    
        if (!marshalling()->readProperties(_propertyfile)) {
            log(Error) << "Failed to read the property file, continueing with default values." << endlog();
        } 
       
#if (defined (OROPKG_OS_LXRT))
        int encoderOffsets[KUKA361_NUM_AXES] = KUKA361_ENCODEROFFSETS;
        
        _comediDev        = new ComediDevice( 1 );
        _comediSubdevAOut = new ComediSubDeviceAOut( _comediDev, "Kuka361" );
        _apci1710         = new EncoderSSI_apci1710_board( 0, 1 );
        _apci2200         = new RelayCardapci2200( "Kuka361" );
        _apci1032         = new SwitchDigitalInapci1032( "Kuka361" );
        _comediDev_NI6024  = new ComediDevice( 4 ); 
        _comediSubdevAIn_NI6024  = new ComediSubDeviceAIn( _comediDev_NI6024, "Kuka361", 0 ); 
        _comediSubdevDIn_NI6024  = new ComediSubDeviceDIn( _comediDev_NI6024, "Kuka361", 2 ); 
        _comediDev_NI6527  = new ComediDevice( 3 ); 
        _comediSubdevDOut_NI6527  = new ComediSubDeviceDOut( _comediDev_NI6527, "Kuka361", 1 ); 
        
        

        for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++){
            //Setting up encoders
            _encoderInterface[i] = new EncoderSSI_apci1710( i + 1, _apci1710 );
            _encoder[i]          = new AbsoluteEncoderSensor( _encoderInterface[i], 1.0 / ticks2rad[i], encoderOffsets[i], -10, 10 );

            _brake[i] = new DigitalOutput( _apci2200, i + KUKA361_NUM_AXES );
            log(Info)<<"Setting brake "<<i<<" On."<<endlog();
            _brake[i]->switchOn();

           _tachoInput[i] = new AnalogInput<unsigned int>(_comediSubdevAIn_NI6024, i+TACHO_OFFSET); 
           unsigned int range = 0; // The input range is -10 to 10 V, so range 0 
           _comediSubdevAIn_NI6024->rangeSet(i+TACHO_OFFSET, range); 
           _comediSubdevAIn_NI6024->arefSet(i+TACHO_OFFSET, AnalogInInterface<unsigned int>::Common); 
           _tachometer[i] = new AnalogSensor( _tachoInput[i], _comediSubdevAIn_NI6024->lowest(i+TACHO_OFFSET), _comediSubdevAIn_NI6024->highest(i+TACHO_OFFSET), _tachoConvertScale[i], _tachoConvertOffset[i]); 

            _currentInput[i] = new AnalogInput<unsigned int>(_comediSubdevAIn_NI6024, i+CURRENT_OFFSET); 
            range = 1; // for a input range -5 to 5 V, range is 1 
            _comediSubdevAIn_NI6024->rangeSet(i+CURRENT_OFFSET, range); 
            _comediSubdevAIn_NI6024->arefSet(i+CURRENT_OFFSET, AnalogInInterface<unsigned int>::Common); 
            _currentSensor[i] = new AnalogSensor( _currentInput[i], _comediSubdevAIn_NI6024->lowest(i+CURRENT_OFFSET), _comediSubdevAIn_NI6024->highest(i+CURRENT_OFFSET), 1.0 , 0); // 1.0 / _shunt_R[i]

            _modeSwitch[i] = new DigitalOutput( _comediSubdevDOut_NI6527, i+MODE_OFFSET ); //Velocity or torque control, selected by relay board 
//            _modeCheck[i] = new DigitalInput( _comediSubdevDIn_NI6024, i ); 

            _ref[i]   = new AnalogOutput<unsigned int>( _comediSubdevAOut, i );
            _enable[i] = new DigitalOutput( _apci2200, i );

            //Put mode back to velocitycontrol
            _modeSwitch[i]->switchOff();

            if ( _mode.value()[i] ){
                     _modeSwitch[i]->switchOn(); 
//                     if ( !_modeCheck[i]->isOn() ) {
//                                log(Error) << "Failed to switch relay of channel " << i << " to torque control mode" << endlog(); 
//                     }
                     _drive[i]  = new AnalogDrive( _ref[i], _enable[i], _curReg_a[i] / _shunt_R[i], - _shunt_c[i] - (_curReg_b[i] / _shunt_R[i])); 
            }
            else{
                     _drive[i]  = new AnalogDrive( _ref[i], _enable[i], 1.0 / vel2volt[i], _velDriveOffset.value()[i]);
            }
      
            _axes_hardware[i] = new RTT::Axis( _drive[i] );
            _axes_hardware[i]->setBrake( _brake[i] );
            _axes_hardware[i]->setSensor( "Position", _encoder[i] );
            _axes_hardware[i]->setSensor( "Velocity", _tachometer[i] ); 
            _axes_hardware[i]->setSensor( "Current", _currentSensor[i] ); 
//            _axes_hardware[i]->setSwitch( "Mode", _modeCheck[i] ); 

            if ( _mode.value()[i] ){
                     _axes_hardware[i]->limitDrive(-_currentLimits.value()[i], _currentLimits.value()[i], _currentOutOfRange); 
            }else{
                     _axes_hardware[i]->limitDrive(-_velocityLimits.value()[i], _velocityLimits.value()[i], _velocityOutOfRange);
            }
        }


// 	//Temp for motor current measurements 
//         _motorCurrentInput = new AnalogInput<unsigned int>(_comediSubdevAIn_NI6024, 0); 
//         unsigned int range2 = 1; // for a input range -5 to 5 V, range is 1 
//         _comediSubdevAIn_NI6024->rangeSet(0, range2); 
//         _comediSubdevAIn_NI6024->arefSet(0, AnalogInInterface<unsigned int>::Common); 
//         _motorCurrentSensor = new AnalogSensor( _motorCurrentInput, _comediSubdevAIn_NI6024->lowest(1), _comediSubdevAIn_NI6024->highest(1), 1.0 , 0);

        
#endif
        for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++){
             if ( _mode.value()[i] ) {
                   _axes_simulation[i] = new RTT::TorqueSimulationAxis(_initialPosition.value()[i], _lowerPositionLimits.value()[i], _upperPositionLimits.value()[i],_velocityLimits.value()[i]);
            } else {
                   _axes_simulation[i] = new RTT::SimulationAxis(_initialPosition.value()[i], _lowerPositionLimits.value()[i], _upperPositionLimits.value()[i]);
             }
        }

        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
	    _pos_sim[i] = _initialPosition.value()[i];
        }
	_torqueSimulator = new Kuka361TorqueSimulator(_axes_simulation, _pos_sim);

#if (defined (OROPKG_OS_LXRT))
        if(!_simulation.value()){
            for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++)
                _axes[i] = _axes_hardware[i];
            log(Info) << "LXRT version of LiASnAxesVelocityController has started" << endlog();
        }
        else{
            for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++)
                _axes[i] = _axes_simulation[i];
            log(Info) << "LXRT simulation version of Kuka361nAxesTorqueController has started" << endlog();
        }
#else
        for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++)
            _axes[i] = _axes_simulation[i];
        log(Info) << "GNULINUX simulation version of Kuka361nAxesTorqueController has started" << endlog();
#endif
        
        // make task context
        /*
         *  Command Interface
         */
        this->methods()->addMethod( &_startAxis, "start axis, starts updating the drive-value (only possible after unlockAxis)","axis","axis to start" );
        this->methods()->addMethod( &_stopAxis,"stop axis, sets drive value to zero and disables the update of the drive-port, (only possible if axis is started","axis","axis to stop");
        this->methods()->addMethod( &_lockAxis,"lock axis, enables the brakes (only possible if axis is stopped","axis","axis to lock" );
        this->methods()->addMethod( &_unlockAxis,"unlock axis, disables the brakes and enables the drive (only possible if axis is locked","axis","axis to unlock" );
        this->methods()->addMethod( &_startAllAxes, "start all axes"  );
        this->methods()->addMethod( &_stopAllAxes, "stops all axes"  );
        this->methods()->addMethod( &_lockAllAxes, "locks all axes"  );
        this->methods()->addMethod( &_unlockAllAxes, "unlock all axes"  );
        this->commands()->addCommand( &_prepareForUse, "prepares the robot for use"  );
        this->commands()->addCommand( &_prepareForShutdown,"prepares the robot for shutdown"  );
        this->methods()->addMethod( &_addDriveOffset,"adds an offset to the drive value of axis","axis","axis to add offset to","offset","offset value in rad/s" );


        /**
         * Creating and adding the data-ports
         */
        for (int i=0;i<KUKA361_NUM_AXES;++i) {
            char buf[80];
            sprintf(buf,"driveValue%d",i);
            _driveValue[i] = new ReadDataPort<double>(buf);
            ports()->addPort(_driveValue[i]);
            sprintf(buf,"positionValue%d",i);
            _positionValue[i]  = new DataPort<double>(buf);
            ports()->addPort(_positionValue[i]);
            sprintf(buf,"velocityValue%d",i); 
            _velocityValue[i]  = new DataPort<double>(buf); 
            ports()->addPort(_velocityValue[i]); 
            sprintf(buf,"currentValue%d",i);
            _currentValue[i]  = new DataPort<double>(buf); 
            ports()->addPort(_currentValue[i]); 
        }
// 	//Temp
//         _motorCurrentValue  = new DataPort<double>("motorCurrentValue"); 
//         ports()->addPort(_motorCurrentValue); 
        
    /**
     * Adding the events :
     */
    events()->addEvent( &_velocityOutOfRange, "Velocity of an Axis is out of range","message","Information about event" );
    events()->addEvent( &_positionOutOfRange, "Position of an Axis is out of range","message","Information about event");
    events()->addEvent( &_currentOutOfRange, "Current of an Axis is out of range","message","Information about event"); 
  }
    
    Kuka361nAxesTorqueController::~Kuka361nAxesTorqueController()
    {
        // make sure robot is shut down
        prepareForShutdown();
    
        // brake, drive, sensors and switches are deleted by each axis
        for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++)
            delete _axes_simulation[i];
    
#if (defined (OROPKG_OS_LXRT))
        for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++)
        delete _axes_hardware[i];
        delete _comediDev;
        delete _comediSubdevAOut;
        delete _apci1710;
        delete _apci2200;
        delete _apci1032;
        delete _comediDev_NI6024; 
        delete _comediSubdevAIn_NI6024; 
        delete _comediSubdevDIn_NI6024; 
        delete _comediDev_NI6527; 
        delete _comediSubdevDOut_NI6527; 
#endif
    }
  
  
    bool Kuka361nAxesTorqueController::startup()
    {
        return true;
    }
  
    void Kuka361nAxesTorqueController::update()
    {

#if (defined (OROPKG_OS_LXRT)) 
        if(_simulation.rvalue()) {
            //simulate kuka 361 in torque mode
            for (int axis=0;axis<KUKA361_NUM_AXES;axis++) {
                _tau_sim[axis] = _driveValue[axis]->Get();
            }
            _torqueSimulator->update(_tau_sim);
        }
#else
            //simulate kuka 361
            for (int axis=0;axis<KUKA361_NUM_AXES;axis++) {
                _tau_sim[axis] = _driveValue[axis]->Get();
            }
            _torqueSimulator->update(_tau_sim);
#endif

            for (int axis=0;axis<KUKA361_NUM_AXES;axis++) {
                // Set the velocity and the position and perform checks in joint space.
                //_velocityValue[axis]->Set(_axes[axis]->getSensor("Velocity")->readSensor()); 
                //vel=(pos-prevpos)/dt
                TimeService::Seconds _delta_time = TimeService::Instance()->secondsSince(_previous_time[axis]);
                _velocityValue[axis]->Set((_axes[axis]->getSensor("Position")->readSensor() - _positionValue[axis]->Get())/_delta_time);
                _previous_time[axis] = TimeService::Instance()->getTicks();
                _positionValue[axis]->Set(_axes[axis]->getSensor("Position")->readSensor());
                           
                // emit event when velocity is out of range
                if( (_velocityValue[axis]->Get() < -_velocityLimits.value()[axis]) ||
                    (_velocityValue[axis]->Get() > _velocityLimits.value()[axis]) ){
                    _velocityOutOfRange("Velocity of a Kuka361 Axis is out of range");
                //log(Warning) <<"velocity"<<_velocityValue[axis]->Get()<<endlog();
                //log(Warning) <<"velsensor: "<<_axes[axis]->getSensor("Velocity")->readSensor()<<endlog();
                //log(Warning) <<"axis: "<<axis<<endlog();
		}
                
                // emit event when position is out of range
                if( (_positionValue[axis]->Get() < _lowerPositionLimits.value()[axis]) ||
                    (_positionValue[axis]->Get() > _upperPositionLimits.value()[axis]) )
                    _positionOutOfRange("Position  of a Kuka361 Axis is out of range");
                
                // send the drive value to hw and performs checks, convert torque to current if torque controlled
                if (_axes[axis]->isDriven()) {
                    if( _mode.value()[axis] ){
                        _axes[axis]->drive(_driveValue[axis]->Get() / _Km[axis]); // accepts a current 
                        //_axes[axis]->drive(_driveValue[axis]->Get());
                    }
                    else
                        _axes[axis]->drive(_driveValue[axis]->Get());
                }
                
                // Set the measured current
		if( _mode.value()[axis] ){
                	_currentValue[axis]->Set(_axes[axis]->getSensor("Current")->readSensor() / _shunt_R[axis] + _shunt_c[axis]);
		}		
            }
// 		//temp
// 		_motorCurrentValue->Set(_motorCurrentSensor->readSensor());
            
            //log(Debug) <<"pos (rad): "<<_positionValue[3]->Get()<<" | vel (rad/s): "<<_velocityValue[3]->Get()<<" | drive (A): "<<_driveValue[3]->Get()<<" | cur (A): "<<_currentValue[3]->Get()<<endlog();
     }

    void Kuka361nAxesTorqueController::shutdown()
    {
        //Make sure machine is shut down
        prepareForShutdown();
        //Write properties back to file
        marshalling()->writeProperties(_propertyfile);
    }
  
    
    bool Kuka361nAxesTorqueController::prepareForUse()
    {
#if (defined (OROPKG_OS_LXRT))
        if(!_simulation.value()){
            _apci2200->switchOn( 12 );
            _apci2200->switchOn( 14 );
            log(Warning) <<"Release Emergency stop and push button to start ..."<<endlog();
        }
#endif
        _activated = true;
        return true;
    }
    
    bool Kuka361nAxesTorqueController::prepareForUseCompleted()const
    {
#if (defined (OROPKG_OS_LXRT))
        if(!_simulation.rvalue())
            return (_apci1032->isOn(12) && _apci1032->isOn(14));
        else
#endif
            return true;
    }
    
    bool Kuka361nAxesTorqueController::prepareForShutdown()
    {
        //make sure all axes are stopped and locked
        stopAllAxes();
        lockAllAxes();
#if (defined (OROPKG_OS_LXRT))
        //Put mode back to velocitycontrol
        for(unsigned int i=0;i<KUKA361_NUM_AXES;i++)
            _modeSwitch[i]->switchOff();
        if(!_simulation.value()){
            _apci2200->switchOff( 12 );
            _apci2200->switchOff( 14 );
        }
        
#endif
        _activated = false;
        return true;
    }
    
    bool Kuka361nAxesTorqueController::prepareForShutdownCompleted()const
    {
        return true;
    }
  
    bool Kuka361nAxesTorqueController::stopAxisCompleted(int axis)const
    {
        return _axes[axis]->isStopped();
    }
  
    bool Kuka361nAxesTorqueController::lockAxisCompleted(int axis)const
    {
        return _axes[axis]->isLocked();
    }
  
    bool Kuka361nAxesTorqueController::startAxisCompleted(int axis)const
    {
        return _axes[axis]->isDriven();
    }
  
    bool Kuka361nAxesTorqueController::unlockAxisCompleted(int axis)const
    {
        return !_axes[axis]->isLocked();
    }
  
    bool Kuka361nAxesTorqueController::stopAxis(int axis)
    {
        if (!(axis<0 || axis>KUKA361_NUM_AXES-1)){
            return _axes[axis]->stop();
	}
        else{
          log(Error) <<"Axis "<< axis <<"doesn't exist!!"<<endlog();
          return false;
        }
    }
    
    bool Kuka361nAxesTorqueController::startAxis(int axis)
    {
        if (!(axis<0 || axis>KUKA361_NUM_AXES-1))
            return _axes[axis]->drive(0.0);
        else{
            log(Error) <<"Axis "<< axis <<"doesn't exist!!"<<endlog();
            return false;
        }
    }
  
    bool Kuka361nAxesTorqueController::unlockAxis(int axis)
    {
        if(_activated){
            if (!(axis<0 || axis>KUKA361_NUM_AXES-1))
                return _axes[axis]->unlock();
            else{
                log(Error) <<"Axis "<< axis <<"doesn't exist!!"<<endlog();
                return false;
            }
        }
        else
            return false;
    }
  
    bool Kuka361nAxesTorqueController::lockAxis(int axis)
    {
        if (!(axis<0 || axis>KUKA361_NUM_AXES-1))
            return _axes[axis]->lock();
        else{
            log(Error) <<"Axis "<< axis <<"doesn't exist!!"<<endlog();
            return false;
        }
    }
  
    bool Kuka361nAxesTorqueController::stopAllAxes()
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            _return &= stopAxis(i);
        }
        return _return;
    }
  
    bool Kuka361nAxesTorqueController::startAllAxes()
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            _return &= startAxis(i);
        }
        return _return;
    }
  
    bool Kuka361nAxesTorqueController::unlockAllAxes()
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            _return &= unlockAxis(i);
        }
        return _return;
    }
  
    bool Kuka361nAxesTorqueController::lockAllAxes()
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            _return &= lockAxis(i);
        }
        return _return;
    }
  
    bool Kuka361nAxesTorqueController::stopAllAxesCompleted()const
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++)
            _return &= stopAxisCompleted(i);
        return _return;
    }
  
    bool Kuka361nAxesTorqueController::startAllAxesCompleted()const
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++)
            _return &= startAxisCompleted(i);
        return _return;
    }
  
    bool Kuka361nAxesTorqueController::lockAllAxesCompleted()const
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++)
            _return &= lockAxisCompleted(i);
        return _return;
    }
  
    bool Kuka361nAxesTorqueController::unlockAllAxesCompleted()const
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++)
            _return &= unlockAxisCompleted(i);
        return _return;
    }
  
    bool Kuka361nAxesTorqueController::addDriveOffset(int axis, double offset)
    {
        _velDriveOffset.value()[axis] += offset;
        
#if (defined (OROPKG_OS_LXRT))
        if (!_simulation.value())
            ((Axis*)(_axes[axis]))->getDrive()->addOffset(offset);
#endif
        return true;
    }
  
    bool Kuka361nAxesTorqueController::addDriveOffsetCompleted(int axis)const
    {
        return true;
    }
    
}//namespace orocos
