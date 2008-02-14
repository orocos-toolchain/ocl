// Copyright  (C)  2007  Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// Version: 1.0
// Author: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// Maintainer: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// URL: http://www.orocos.org/ocl

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Servo controller based on work of CRSnAxesVelocityContoller by
// Erwin Aertbelien

#include "StaubliRX130nAxesVelocityController.hpp"
#include <ocl/ComponentLoader.hpp>

ORO_CREATE_COMPONENT( OCL::StaubliRX130nAxesVelocityController );

#include <rtt/Logger.hpp>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <bitset>

namespace OCL
{
    using namespace RTT;
    using namespace KDL;
    using namespace std;
    
#define STAUBLIRX130_NUM_AXES 6
  
#define STAUBLIRX130_ENCODEROFFSETS { 0, 0, 0, 0, 0, 0 }
  
#define STAUBLIRX130_CONV1  -128.6107
#define STAUBLIRX130_CONV2  -129.3939
#define STAUBLIRX130_CONV3  101.0909
#define STAUBLIRX130_CONV4  88
#define STAUBLIRX130_CONV5  75
#define STAUBLIRX130_CONV6  32
  
#define STAUBLIRX130_ENC_RES  4096

    // Conversion from encoder ticks to radiants
#define STAUBLIRX130_TICKS2RAD { 2*M_PI / (STAUBLIRX130_CONV1 * STAUBLIRX130_ENC_RES), 2*M_PI / (STAUBLIRX130_CONV2 * STAUBLIRX130_ENC_RES), 2*M_PI / (STAUBLIRX130_CONV3 * STAUBLIRX130_ENC_RES), 2*M_PI / (STAUBLIRX130_CONV4 * STAUBLIRX130_ENC_RES), 2*M_PI / (STAUBLIRX130_CONV5 * STAUBLIRX130_ENC_RES), 2*M_PI / (STAUBLIRX130_CONV6 * STAUBLIRX130_ENC_RES)}
    
  // Conversion from angular speed to voltage
#define STAUBLIRX130_RADproSEC2VOLT { -2.9452, -2.9452, 1.25, -1.28, 1.15, 0.475 }
#define DRIVEVALUE_5_TO_6 (1.15/0.475)
  
#define KINEMATICS_EPS 0.0001
#define SQRT3d2 0.8660254037844386
#define M_PI_T2 2 * M_PI
#define SQRT3t2 3.46410161513775 // 2 sqrt(3)
  
    typedef StaubliRX130nAxesVelocityController MyType;
    
    StaubliRX130nAxesVelocityController::StaubliRX130nAxesVelocityController(string name)
        : TaskContext(name,PreOperational),
          startAllAxes_mtd( "startAllAxes", &MyType::startAllAxes, this),
          stopAllAxes_mtd( "stopAllAxes", &MyType::stopAllAxes, this),
          unlockAllAxes_mtd( "unlockAllAxes", &MyType::unlockAllAxes, this),
          lockAllAxes_mtd( "lockAllAxes", &MyType::lockAllAxes, this),
          prepareForUse_cmd( "prepareForUse", &MyType::prepareForUse,&MyType::prepareForUseCompleted, this),
          prepareForShutdown_cmd( "prepareForShutdown", &MyType::prepareForShutdown,&MyType::prepareForShutdownCompleted, this),
          addDriveOffset_mtd( "addDriveOffset", &MyType::addDriveOffset, this),
          driveValues_port("nAxesOutputVelocity"),
          servoValues_port("nAxesServoVelocity"),
          positionValues_port("nAxesSensorPosition"),
          velocityValues_port("nAxesSensorVelocity"),
          deltaTime_port("DeltaTime"),
          driveLimits_prop("driveLimits","velocity limits of the axes, (rad/s)",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          lowerPositionLimits_prop("LowerPositionLimits","Lower position limits (rad)",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          upperPositionLimits_prop("UpperPositionLimits","Upper position limits (rad)",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          velocityLimits_prop("velocityLimits","velocity limits of the axes, (rad/s)",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          initialPosition_prop("initialPosition","Initial position (rad) for simulation or hardware",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          driveOffset_prop("driveOffset","offset (in rad/s) to the drive value.",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          simulation_prop("simulation","true if simulationAxes should be used",true),
          simulation(true),
          num_axes_attr("NUM_AXES",STAUBLIRX130_NUM_AXES),
          chain_attr("Kinematics"),
          driveOutOfRange_evt("driveOutOfRange"),
          positionOutOfRange_evt("positionOutOfRange"),
          velocityOutOfRange_evt("velocityOutOfRange"),
          servoIntegrationFactor_prop("ServoIntegrationFactor","Inverse of Integration time for servoloop",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          servoGain_prop("ServoGain","Feedback Gain for servoloop",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          servoFFScale_prop("ServoFFScale","Feedforward scale for servoloop",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          EmergencyEvents_prop("EmergencyEvents","List of events that will result in an emergencystop of the robot"),
          activated(false),
          positionConvertFactor(STAUBLIRX130_NUM_AXES,0),
          driveConvertFactor(STAUBLIRX130_NUM_AXES,0),
          positionValues(STAUBLIRX130_NUM_AXES,0),
          driveValues(STAUBLIRX130_NUM_AXES,0),
          velocityValues(STAUBLIRX130_NUM_AXES,0),
          servoIntError(STAUBLIRX130_NUM_AXES,0),
          outputvel(STAUBLIRX130_NUM_AXES,0),
          servoIntegrationFactor(STAUBLIRX130_NUM_AXES,0),
          servoGain(STAUBLIRX130_NUM_AXES,0),
          servoFFScale(STAUBLIRX130_NUM_AXES,0),
          servoInitialized(false),
#if (defined OROPKG_OS_LXRT)
          axes_hardware(STAUBLIRX130_NUM_AXES),
          encoderInterface(STAUBLIRX130_NUM_AXES),
          encoder(STAUBLIRX130_NUM_AXES),
          vref(STAUBLIRX130_NUM_AXES),
          enable(STAUBLIRX130_NUM_AXES),
          drive(STAUBLIRX130_NUM_AXES),
          driveFailure(STAUBLIRX130_NUM_AXES),
#endif
          axes(STAUBLIRX130_NUM_AXES),
          _previous_time(STAUBLIRX130_NUM_AXES)
    {
        Logger::In in(this->getName().data());
        double ticks2rad[STAUBLIRX130_NUM_AXES] = STAUBLIRX130_TICKS2RAD;
        double vel2volt[STAUBLIRX130_NUM_AXES] = STAUBLIRX130_RADproSEC2VOLT;
        for(unsigned int i = 0;i<STAUBLIRX130_NUM_AXES;i++){
            positionConvertFactor[i] = ticks2rad[i];
            driveConvertFactor[i] = vel2volt[i];
        }
        
#if (defined OROPKG_OS_LXRT)
        double encoderOffsets[STAUBLIRX130_NUM_AXES] = STAUBLIRX130_ENCODEROFFSETS;
        
        log(Info)<<"Creating Comedi Devices."<<endlog();
        AOut = new ComediDevice(0);
        DInOut = new ComediDevice(1);
        Encoder = new ComediDevice(2);
        
        SubAOut = new ComediSubDeviceAOut(AOut,"AnalogOut",1);
        SubDIn = new ComediSubDeviceDIn(DInOut,"DigitalIn",0);
        SubDOut = new ComediSubDeviceDOut(DInOut,"DigitalOut",1);
        
        brakeOff = new DigitalOutput(SubDOut,17);
        brakeOff->switchOff();
        
        highPowerEnable = new DigitalOutput(SubDOut,16);
        
        eStop = new DigitalInput(SubDIn,14);
        
        armPowerOn = new DigitalInput(SubDIn,15);
        

        for (unsigned int i = 0; i < STAUBLIRX130_NUM_AXES; i++){
            log(Info)<<"Creating Hardware axis "<<i<<endlog();
            //Setting up encoders
            log(Info)<<"Setting up encoder ..."<<endlog();
            encoderInterface[i] = new ComediEncoder(Encoder,0,i);
            encoder[i] = new IncrementalEncoderSensor( encoderInterface[i], 1.0 / ticks2rad[i],
                                                       encoderOffsets[i],
                                                       -10, 10,STAUBLIRX130_ENC_RES );
            
            log(Info)<<"Setting up drive ..."<<endlog();
            log(Debug)<<"Creating AnalogOutput"<<endlog();
            vref[i]   = new AnalogOutput<unsigned int>(SubAOut, i );
            log(Debug)<<"Creating Digital Output Enable"<<endlog();
            enable[i] = new DigitalOutput( SubDOut, 18+i );
            log(Debug)<<"Initial switch off brake"<<endlog();
            enable[i]->switchOff();
            log(Debug)<<"Creating AnalogDrive"<<endlog();
            drive[i]  = new AnalogDrive( vref[i], enable[i], 1.0 / vel2volt[i], 0.0);
            
            log(Debug)<<"Creating Digital Input DriveFailure"<<endlog();
            driveFailure[i] = new DigitalInput(SubDIn,8+i);
            log(Debug)<<"Creating Axis"<<endlog();
            axes_hardware[i] = new Axis( drive[i] );
            log(Debug)<<"Adding Position Sensor to Axis"<<endlog();
            axes_hardware[i]->setSensor( "Position", encoder[i] );
        }
        
        
#endif
        //Definition of kinematics for the StaubliRX130 
        kinematics.addSegment(Segment(Joint(Joint::RotZ),Frame(Rotation::RotZ(M_PI_2),Vector(0.0,0.0,0.2075+0.550))));
        kinematics.addSegment(Segment(Joint(Joint::RotX),Frame(Vector(0.0,0.0,0.625))));
        kinematics.addSegment(Segment(Joint(Joint::RotX),Frame(Vector(0.0,0.0,0.625))));
        kinematics.addSegment(Segment(Joint(Joint::RotZ)));
        kinematics.addSegment(Segment(Joint(Joint::RotX),Frame(Vector(0.0,0.0,0.110))));
        kinematics.addSegment(Segment(Joint(Joint::RotZ),Frame(Rotation::RotZ(-M_PI_2))));
        
        chain_attr.set(kinematics);
        
        /*
         *  Execution Interface
         */
        methods()->addMethod( &startAllAxes_mtd, "start all axes"  );
        methods()->addMethod( &stopAllAxes_mtd, "stops all axes"  );
        methods()->addMethod( &lockAllAxes_mtd, "locks all axes"  );
        methods()->addMethod( &unlockAllAxes_mtd, "unlock all axes"  );
        commands()->addCommand( &prepareForUse_cmd, "prepares the robot for use"  );
        commands()->addCommand( &prepareForShutdown_cmd,"prepares the robot for shutdown"  );
        methods()->addMethod( &addDriveOffset_mtd,"adds an offset to the drive value of axis","offset","offset values in rad/s" );
        events()->addEvent( &driveOutOfRange_evt, "Drive value of an Axis is out of range","message","Information about event" );
        events()->addEvent( &positionOutOfRange_evt, "Position of an Axis is out of range","message","Information about event");
        events()->addEvent( &velocityOutOfRange_evt, "Velocity of an Axis is out of range","message","Information about event");
        
        /**
         * Dataflow Interface
         */
        ports()->addPort(&driveValues_port);
        ports()->addPort(&servoValues_port);
        ports()->addPort(&positionValues_port);
        ports()->addPort(&velocityValues_port);
        ports()->addPort(&deltaTime_port);
        
        /**
         * Configuration Interface
         */
        properties()->addProperty( &driveLimits_prop);
        properties()->addProperty( &lowerPositionLimits_prop);
        properties()->addProperty( &upperPositionLimits_prop);
        properties()->addProperty( &velocityLimits_prop);
        properties()->addProperty( &initialPosition_prop);
        properties()->addProperty( &driveOffset_prop);
        properties()->addProperty( &simulation_prop);
        properties()->addProperty( &servoIntegrationFactor_prop);
        properties()->addProperty( &servoGain_prop);
        properties()->addProperty( &servoFFScale_prop);
        properties()->addProperty( &EmergencyEvents_prop);
        attributes()->addConstant( &num_axes_attr);
        attributes()->addAttribute(&chain_attr);
        
    }
    
    StaubliRX130nAxesVelocityController::~StaubliRX130nAxesVelocityController()
    {
        // make sure robot is shut down
        prepareForShutdown_cmd();
        // make sure last position is stored
        this->cleanupHook();
        
        // brake, drive, sensors and switches are deleted by each axis
        if(simulation_prop.value())
            for (unsigned int i = 0; i < STAUBLIRX130_NUM_AXES; i++)
                delete axes[i];
    
#if (defined OROPKG_OS_LXRT)
        for (unsigned int i = 0; i < STAUBLIRX130_NUM_AXES; i++){
            delete axes_hardware[i];
            delete driveFailure[i];
        }
        delete armPowerOn;
        delete eStop;
        delete highPowerEnable;
        delete brakeOff;
        
        delete SubAOut;
        delete SubDIn;
        delete SubDOut;
        
        delete AOut;
        delete DInOut;
        delete Encoder;
#endif
    }
    
    bool StaubliRX130nAxesVelocityController::configureHook()
    {
        Logger::In in(this->getName().data());
        
        simulation=simulation_prop.value();
        
        if(!(driveLimits_prop.value().size()==STAUBLIRX130_NUM_AXES&&
             lowerPositionLimits_prop.value().size()==STAUBLIRX130_NUM_AXES&&
             upperPositionLimits_prop.value().size()==STAUBLIRX130_NUM_AXES&&
             velocityLimits_prop.value().size()==STAUBLIRX130_NUM_AXES&&
             initialPosition_prop.value().size()==STAUBLIRX130_NUM_AXES&&
             driveOffset_prop.value().size()==STAUBLIRX130_NUM_AXES))
            return false;
        
#if (defined OROPKG_OS_LXRT)
        if(!simulation){
            
            for (unsigned int i = 0; i <STAUBLIRX130_NUM_AXES; i++){
                axes_hardware[i]->limitDrive(-driveLimits_prop.value()[i], driveLimits_prop.value()[i], driveOutOfRange_evt);
                axes[i] = axes_hardware[i];
                ((Axis*)(axes[i]))->getDrive()->addOffset(driveOffset_prop.value()[i]);
                log(Info) << "Hardware version of StaubliRX130nAxesVelocityController has started" << endlog();
            }
            
            std::vector<double> initPos(STAUBLIRX130_NUM_AXES,0.0);
            if(!this->readAbsolutePosition(initPos))
                return false;
            else
                for(unsigned int i=0;i<STAUBLIRX130_NUM_AXES;i++)
                    if(i!=5)
                        ((IncrementalEncoderSensor*)(axes[i]->getSensor("Position")))->writeSensor(initPos[i]);
                    else
                        ((IncrementalEncoderSensor*)(axes[i]->getSensor("Position")))->writeSensor(initPos[i]+initPos[i-1]);
        }
        else{
#endif
            for (unsigned int i = 0; i <STAUBLIRX130_NUM_AXES; i++)
                axes[i] = new SimulationAxis(initialPosition_prop.value()[i],
                                             lowerPositionLimits_prop.value()[i],
                                             upperPositionLimits_prop.value()[i]);
            log(Info) << "Simulation version of StaubliRX130nAxesVelocityController has started" << endlog();
#if (defined OROPKG_OS_LXRT)
        }
#endif
        /**
         * Initializing servoloop
         */
        servoGain = servoGain_prop.value();
        servoIntegrationFactor = servoIntegrationFactor_prop.value();
        servoFFScale = servoFFScale_prop.value();
	
        /**
         * Creating event handlers
         */
        log(Info)<<"Creating EmergencyEvent Handlers"<<endlog();
        
        for(vector<string>::const_iterator it=EmergencyEvents_prop.rvalue().begin();it!=EmergencyEvents_prop.rvalue().end();it++){
            string::size_type idx = (*it).find('.');
            if(idx==string::npos)
                log(Warning)<<"Could not connect EmergencyStop to "<<(*it)<<"\n Syntax of "
                            <<(*it)<<" is not correct. I want a ComponentName.EventName "<<endlog();
            string peername = (*it).substr(0,idx);
            string eventname = (*it).substr(idx+1);
            TaskContext* peer;
            if(peername==this->getName())
                peer = this;
            else if(this->hasPeer(peername)){
                peer = this->getPeer(peername);
            }else{
                log(Warning)<<"Could not connect EmergencyStop to "<<(*it)<<", "<<peername<<" is not a peer of "<<this->getName()<<endlog();
                continue;
            }
            
            if(peer->events()->hasEvent(eventname)){
                Handle handle = peer->events()->setupConnection(eventname).callback(this,&StaubliRX130nAxesVelocityController::EmergencyStop).handle();
                if(handle.connect()){
                    EmergencyEventHandlers.push_back(handle);
                    log(Info)<<"EmergencyStop connected to "<< (*it)<<" event."<<endlog();
                }else
                    log(Warning)<<"Could not connect EmergencyStop to "<<(*it)<<", "<<eventname<<" has to have a message parameter."<<endlog();
            }else
                log(Warning)<<"Could not connect EmergencyStop to "<<(*it)<<", "<<eventname <<" not found in "<<peername<<"s event-list"<<endlog();
        }
        
        return true;
    }
      
    bool StaubliRX130nAxesVelocityController::startHook()
    {
        for (int axis=0;axis<STAUBLIRX130_NUM_AXES;axis++) {
#if defined OROPKG_OS_LXRT
            if (axis != STAUBLIRX130_NUM_AXES-1||simulation){
#endif
      	         	_previous_time[axis] = TimeService::Instance()->getTicks();
                    positionValues[axis] = axes[axis]->getSensor("Position")->readSensor();
#if defined OROPKG_OS_LXRT
            }else{
                //Last Axes moves together with previous axes
                _previous_time[axis] = TimeService::Instance()->getTicks();
		        positionValues[axis] = axes[axis]->getSensor("Position")->readSensor() - axes[axis-1]->getSensor("Position")->readSensor();
            }
#endif
        }
        positionValues_port.Set(positionValues);
        return true;
    }
  
    void StaubliRX130nAxesVelocityController::updateHook()
    {
        for (int axis=0;axis<STAUBLIRX130_NUM_AXES;axis++) {
#if defined OROPKG_OS_LXRT
            if (axis != STAUBLIRX130_NUM_AXES-1||simulation){
#endif
                _delta_time = TimeService::Instance()->secondsSince(_previous_time[axis]);
                velocityValues[axis] = (axes[axis]->getSensor("Position")->readSensor() - positionValues[axis])/_delta_time;
                _previous_time[axis] = TimeService::Instance()->getTicks();
                positionValues[axis] = axes[axis]->getSensor("Position")->readSensor();
                deltaTime_port.Set(_delta_time);
#if defined OROPKG_OS_LXRT
            }else{
                //Last Axes moves together with previous axes
                _delta_time = TimeService::Instance()->secondsSince(_previous_time[axis]);
                velocityValues[axis] = (axes[axis]->getSensor("Position")->readSensor() - axes[axis-1]->getSensor("Position")->readSensor() - positionValues[axis])/_delta_time;
                _previous_time[axis] = TimeService::Instance()->getTicks();
                positionValues[axis] = axes[axis]->getSensor("Position")->readSensor() - axes[axis-1]->getSensor("Position")->readSensor();
                deltaTime_port.Set(_delta_time);
            }
#endif
        }
        positionValues_port.Set(positionValues);
        velocityValues_port.Set(velocityValues);
        
#if defined OROPKG_OS_LXRT
        double dt;
        // Determine sampling time :
        if (servoInitialized) {
            dt              = TimeService::Instance()->secondsSince(previousTime);
            previousTime    = TimeService::Instance()->getTicks();
        } else {
            dt = 0.0; 
            for (unsigned int i=0;i<STAUBLIRX130_NUM_AXES;i++) {      
                servoIntError[i] = 0.0;
            }
            previousTime = TimeService::Instance()->getTicks();
            servoInitialized = true;
        }
#endif
        driveValues_port.Get(driveValues);
#if defined OROPKG_OS_LXRT
        // Last axis moves together with second last
        // Substract drive value as a feedforward
        if(!simulation)
            driveValues[5] = driveValues[5] + driveValues[4]/(DRIVEVALUE_5_TO_6);
#endif
        
        for (unsigned int i=0;i<STAUBLIRX130_NUM_AXES;i++) {      
#if defined OROPKG_OS_LXRT
            if (driveFailure[i]->isOn()){
                log(Error)<<"Failure drive "<<i<<", stopping all axes"<<endlog();
                this->fatal();
            }
#endif
            
            // emit event when velocity is out of range
            if( (velocityValues[i] < -velocityLimits_prop.value()[i]) ||
                (velocityValues[i] > velocityLimits_prop.value()[i]) ){
                char msg[80];
                sprintf(msg,"Velocity of StaubliRX130 Axis %d is out of range: %f",i+1,velocityValues[i]);
                velocityOutOfRange_evt(msg);
            }
            
            // emit event when position is out of range
            if( (positionValues[i] < lowerPositionLimits_prop.value()[i]) ||
                (positionValues[i] > upperPositionLimits_prop.value()[i]) ){
                char msg[80];
                sprintf(msg,"Position of StaubliRX130 Axis %d is out of range: %f",i+1,positionValues[i]);
                positionOutOfRange_evt(msg);
            }
            
            // send the drive value to hw and performs checks
            if (axes[i]->isDriven()){
#if defined OROPKG_OS_LXRT      
                // perform control action ( dt is zero the first time !) :
                double error        = driveValues[i] - velocityValues[i];
                servoIntError[i]    += dt*error;
                outputvel[i] = servoGain[i]*(error + servoIntegrationFactor[i]*servoIntError[i]) + servoFFScale[i]*driveValues[i];
            } else {
                outputvel[i] = 0.0;
            }
            if( (outputvel[i] < -velocityLimits_prop.value()[i]) ||
                (outputvel[i] > velocityLimits_prop.value()[i]) ){
                char msg[80];
                sprintf(msg,"Velocity of StaubliRX130 Axis %d is out of range: %f",i+1,velocityValues[i]);
                velocityOutOfRange_evt(msg);
            }
            
            driveValues[i] = outputvel[i];
            axes[i]->drive(driveValues[i]);
        }
               
#else
                axes[i]->drive(driveValues[i]);
            }
        } 

#endif
        servoValues_port.Set(driveValues);
    }
    
    
    void StaubliRX130nAxesVelocityController::stopHook()
    {
        //Make sure machine is shut down
        prepareForShutdown();
    }

    void StaubliRX130nAxesVelocityController::cleanupHook()
    {
        //Write properties back to file
        marshalling()->writeProperties(this->getName()+".cpf");
    }
        
    bool StaubliRX130nAxesVelocityController::prepareForUse()
    {
#if (defined OROPKG_OS_LXRT)
        if(!simulation){
            highPowerEnable->switchOn();
            log(Warning) <<"Release Emergency stop and push button to start ...."<<endlog();
        }
#endif
        activated = true;
        return true;
    }
    
    bool StaubliRX130nAxesVelocityController::prepareForUseCompleted()const
    {
#if (defined OROPKG_OS_LXRT)
        if(!simulation)
            return (!(eStop->isOn()) && armPowerOn->isOn());
        else
#endif
            return true;
    }
    
    bool StaubliRX130nAxesVelocityController::prepareForShutdown()
    {
        //make sure all axes are stopped and locked
        stopAllAxes();
        lockAllAxes();
#if (defined OROPKG_OS_LXRT)
        if(!simulation)
            highPowerEnable->switchOff();
#endif
        activated = false;
        return true;
    }
    
    bool StaubliRX130nAxesVelocityController::prepareForShutdownCompleted()const
    {
        return true;
    }
    
    bool StaubliRX130nAxesVelocityController::stopAllAxes()
    {
        bool succes = true;
        for(unsigned int i = 0;i<STAUBLIRX130_NUM_AXES;i++)
            succes &= axes[i]->stop();
        
        return succes;
    }
    
    bool StaubliRX130nAxesVelocityController::startAllAxes()
    {
        bool succes = true;
        for(unsigned int i = 0;i<STAUBLIRX130_NUM_AXES;i++)
            succes &= axes[i]->drive(0.0);
        
        return succes;
    }
  
    bool StaubliRX130nAxesVelocityController::unlockAllAxes()
    {
        if(!activated)
            return false;
        
        bool succes = true;
        for(unsigned int i = 0;i<STAUBLIRX130_NUM_AXES;i++)
            succes &= axes[i]->unlock();
#if defined OROPKG_OS_LXRT
        if (succes)
            brakeOff->switchOn();
#endif
        return succes;
    }
  
    bool StaubliRX130nAxesVelocityController::lockAllAxes()
    {
        bool succes = true;
#if defined OROPKG_OS_LXRT
        brakeOff->switchOff();
#endif
	for(unsigned int i = 0;i<STAUBLIRX130_NUM_AXES;i++){
            succes &= axes[i]->lock();
        }
        return succes;
    }
    
bool StaubliRX130nAxesVelocityController::addDriveOffset(const vector<double>& offset)
    {
        if(offset.size()!=STAUBLIRX130_NUM_AXES)
            return false;
        
        for(unsigned int i=0;i<STAUBLIRX130_NUM_AXES;i++){
            driveOffset_prop.value()[i] += offset[i];
#if (defined OROPKG_OS_LXRT)
            if (!simulation)
                ((Axis*)(axes[i]))->getDrive()->addOffset(offset[i]);
#endif
        }
        return true;
    }


    bool StaubliRX130nAxesVelocityController::readAbsolutePosition(std::vector<double>& initPos)
    {
#if (defined OROPKG_OS_LXRT)
        //Initialising serial communication
        port="/dev/ttyS0";
        fd=open(port,O_RDWR | O_NOCTTY | O_NDELAY);
        if(fd==-1)
            log(Critical)<<"Could not open serial communication with Adept Controller"<<endlog();
        fcntl(fd,F_SETFL,0);
	  
        if(tcgetattr(fd,&tio)<0){
            log(Error)<<"Serial IO-error!!!"<<endlog();
            return false;
        }
        //setting parameters for communication
        //tio.c_lflag=0; //Non-canonical mode
        tio.c_cflag &= ~PARENB;
        tio.c_cflag &= ~CSTOPB;
        tio.c_cflag &= ~CSIZE;
        tio.c_cflag |= CS8;
        tio.c_cflag|=(CLOCAL | CREAD);
        if(cfsetispeed(&tio,B9600)<0&&cfsetospeed(&tio,B9600)<0){
            log(Error)<<"Could not set serial communication speed"<<endlog();
            return false;
        }
        tio.c_oflag &=~OPOST;
        tio.c_lflag &=~(ICANON|ECHO|ECHOE|ISIG);
        tio.c_cc[VMIN] = 0;
        tio.c_cc[VTIME]=10;
        
        tcflush(fd,TCIFLUSH);
        tcsetattr(fd,TCSANOW,&tio);
        
        char buffer[300];/* Input buffer */
        char *bufptr;/* Current char in buffer */
        int  nbytes;/* Number of bytes read */
        
        //Send CR to see if Adept Controller is ready:
        write(fd,"\r",1);
        bufptr=buffer;
        while((nbytes=read(fd,bufptr,buffer+sizeof(buffer)-bufptr-1))>0){
            bufptr+=nbytes;
            if(bufptr[-1]=='\n'||bufptr[-1]=='\r')
                break;
        }
        /* nul terminate the string and see if we got an OK response */
        *bufptr = '\0';
        log(Debug)<<"Received:"<<buffer<<endlog();
        char dot;
        sscanf(buffer,"%*s%s",&dot);
        if(dot=='.')
            log(Info)<<"Serial communication with Adept Controller OK."<<endlog();
        else{
            log(Error)<<"Serial communication with Adept Controller not ready!"<<endlog();
            return false;
        }
        
        //Get Absolute Position Values
        write(fd,"where\r",7);
        bufptr=buffer;
        unsigned int nlines=0;
        while((nbytes=read(fd,bufptr,buffer+sizeof(buffer)-bufptr-1))>0){
            bufptr+=nbytes;
            if(bufptr[-1]=='\n'||bufptr[-1]=='\r')
                nlines++;
            if(nlines==4)
                break;
        }
        *bufptr = '\0';
        log(Debug)<<"Received:"<<buffer<<endlog();
        sscanf(buffer,"%*s %*s %*s %*s %*s %*s %*s %*s %*f %*f %*f %*f %*f %*f %*f %*s %*s %*s %*s %*s %*s %lf %lf %lf %lf %lf %lf"
               ,&initPos[0],&initPos[1],&initPos[2],&initPos[3],&initPos[4],&initPos[5]);
        log(Info)<<"Absolute joint position in degrees: "<<initPos[0]<<", "<<initPos[1]<<", "<<initPos[2]
                 <<", "<<initPos[3]<<", "<<initPos[4]<<", "<<initPos[5]<<endlog();

        //Adding offsets
        initPos[1]+=90.0;
        initPos[2]-=90.0;
        //Converting to radians
        for(unsigned int i=0;i<initPos.size();i++)
            initPos[i]*=M_PI/180;
#endif
        return true;
    }
    
    
    
    
}//namespace orocos
