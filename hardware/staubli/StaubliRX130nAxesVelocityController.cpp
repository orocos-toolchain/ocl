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

#include "StaubliRX130nAxesVelocityController.hpp"

#include <rtt/Logger.hpp>
#include <stdlib.h>

namespace OCL
{
    using namespace RTT;
    using namespace KDL;
    using namespace std;
  
#define STAUBLIRX130_NUM_AXES 6

#define STAUBLIRX130_ENCODEROFFSETS { 0, 0, 0, 0, 0, 0 }

#define STAUBLIRX130_CONV1  992
#define STAUBLIRX130_CONV2  992
#define STAUBLIRX130_CONV3  101.0909
#define STAUBLIRX130_CONV4  88
#define STAUBLIRX130_CONV5  75
#define STAUBLIRX130_CONV6  32

#define STAUBLIRX130_ENC_RES  4096

    // Conversion from encoder ticks to radiants
#define STAUBLIRX130_TICKS2RAD { 2*M_PI / (STAUBLIRX130_CONV1 * STAUBLIRX130_ENC_RES), 2*M_PI / (STAUBLIRX130_CONV2 * STAUBLIRX130_ENC_RES), 2*M_PI / (STAUBLIRX130_CONV3 * STAUBLIRX130_ENC_RES), 2*M_PI / (STAUBLIRX130_CONV4 * STAUBLIRX130_ENC_RES), 2*M_PI / (STAUBLIRX130_CONV5 * STAUBLIRX130_ENC_RES), 2*M_PI / (STAUBLIRX130_CONV6 * STAUBLIRX130_ENC_RES)}
  
  // Conversion from angular speed to voltage
#define STAUBLIRX130_RADproSEC2VOLT { 1, 1, 1, 1, 1, 1 }

#define KINEMATICS_EPS 0.0001
#define SQRT3d2 0.8660254037844386
#define M_PI_T2 2 * M_PI
#define SQRT3t2 3.46410161513775 // 2 sqrt(3)
    
    typedef StaubliRX130nAxesVelocityController MyType;
    
    StaubliRX130nAxesVelocityController::StaubliRX130nAxesVelocityController(string name,string _propertyfile)
        : TaskContext(name,PreOperational),
          startAllAxes_mtd( "startAllAxes", &MyType::startAllAxes, this),
          stopAllAxes_mtd( "stopAllAxes", &MyType::stopAllAxes, this),
          unlockAllAxes_mtd( "unlockAllAxes", &MyType::unlockAllAxes, this),
          lockAllAxes_mtd( "lockAllAxes", &MyType::lockAllAxes, this),
          prepareForUse_cmd( "prepareForUse", &MyType::prepareForUse,&MyType::prepareForUseCompleted, this),
          prepareForShutdown_cmd( "prepareForShutdown", &MyType::prepareForShutdown,&MyType::prepareForShutdownCompleted, this),
          addDriveOffset_mtd( "addDriveOffset", &MyType::addDriveOffset, this),
          driveValues_port("nAxesOutputVelocity"),
          positionValues_port("nAxesSensorPosition"),
          driveLimits_prop("driveLimits","velocity limits of the axes, (rad/s)",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          lowerPositionLimits_prop("LowerPositionLimits","Lower position limits (rad)",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          upperPositionLimits_prop("UpperPositionLimits","Upper position limits (rad)",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          initialPosition_prop("initialPosition","Initial position (rad) for simulation or hardware",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          driveOffset_prop("driveOffset","offset (in rad/s) to the drive value.",vector<double>(STAUBLIRX130_NUM_AXES,0)),
          simulation_prop("simulation","true if simulationAxes should be used",true),
          simulation(true),
          num_axes_attr("NUM_AXES",STAUBLIRX130_NUM_AXES),
          chain_attr("Kinematics"),
          driveOutOfRange_evt("driveOutOfRange"),
          positionOutOfRange_evt("positionOutOfRange"),
          propertyfile(_propertyfile),
          activated(false),
          positionConvertFactor(STAUBLIRX130_NUM_AXES),
          driveConvertFactor(STAUBLIRX130_NUM_AXES),
          positionValues(STAUBLIRX130_NUM_AXES),
          driveValues(STAUBLIRX130_NUM_AXES),
#if (defined OROPKG_OS_LXRT)
          axes_hardware(STAUBLIRX130_NUM_AXES),
          encoderInterface(STAUBLIRX130_NUM_AXES),
          encoder(STAUBLIRX130_NUM_AXES),
          vref(STAUBLIRX130_NUM_AXES),
          enable(STAUBLIRX130_NUM_AXES),
          drive(STAUBLIRX130_NUM_AXES),
          driveFailure(STAUBLIRX130_NUM_AXES),
#endif
          axes(STAUBLIRX130_NUM_AXES)
    {
        Logger::In in(this->getName().data());
        double ticks2rad[STAUBLIRX130_NUM_AXES] = STAUBLIRX130_TICKS2RAD;
        double vel2volt[STAUBLIRX130_NUM_AXES] = STAUBLIRX130_RADproSEC2VOLT;
        for(unsigned int i = 0;i<STAUBLIRX130_NUM_AXES;i++){
            positionConvertFactor[i] = ticks2rad[i];
            driveConvertFactor[i] = vel2volt[i];
        }

#if (defined OROPKG_OS_LXRT)
        int encoderOffsets[STAUBLIRX130_NUM_AXES] = STAUBLIRX130_ENCODEROFFSETS;
        
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
            encoderInterface[i] = new ComediEncoder( i + 1, apci1710 );
            encoder[i]          = new IncrementalEncoderSensor( encoderInterface[i], 1.0 / ticks2rad[i], encoderOffsets[i], -10, 10 );
                        
            log(Info)<<"Setting up drive ..."<<endlog();
            vref[i]   = new AnalogOutput<unsigned int>( comediSubdevAOut, i );
            enable[i] = new DigitalOutput( SubDOut, 18+i );
            enable[i]->switchOff();
            drive[i]  = new AnalogDrive( vref[i], enable[i], 1.0 / vel2volt[i], 0.0);

            driveFailure[i] = new DigitalInput(SubDIn,8+i);
            axes_hardware[i] = new Axis( drive[i] );
            axes_hardware[i]->setSensor( "Position", encoder[i] );
        }
        
#endif
        //Definition of kinematics for the StaubliRX130 
        kinematics.addSegment(Segment(Joint(Joint::RotZ)));
        kinematics.addSegment(Segment(Joint(Joint::RotX),Frame(Vector(0.0,0.0,1.020))));
        kinematics.addSegment(Segment(Joint(Joint::RotX),Frame(Vector(0.0,0.0,0.480))));
        kinematics.addSegment(Segment(Joint(Joint::RotZ),Frame(Vector(0.0,0.0,0.645))));
        kinematics.addSegment(Segment(Joint(Joint::RotX)));
        kinematics.addSegment(Segment(Joint(Joint::RotZ)));
        kinematics.addSegment(Segment(Joint(Joint::None),Frame(Vector(0.0,0.0,0.120))));
        
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
        events()->addEvent( &driveOutOfRange_evt, "Velocity of an Axis is out of range","message","Information about event" );
        events()->addEvent( &positionOutOfRange_evt, "Position of an Axis is out of range","message","Information about event");
        
        /**
         * Dataflow Interface
         */
        ports()->addPort(&driveValues_port);
        ports()->addPort(&positionValues_port);
        
        /**
         * Configuration Interface
         */
        properties()->addProperty( &driveLimits_prop);
        properties()->addProperty( &lowerPositionLimits_prop);
        properties()->addProperty( &upperPositionLimits_prop);
        properties()->addProperty( &initialPosition_prop);
        properties()->addProperty( &driveOffset_prop);
        properties()->addProperty( &simulation_prop);
        attributes()->addConstant( &num_axes_attr);
        attributes()->addAttribute(&chain_attr);
    }
    
    StaubliRX130nAxesVelocityController::~StaubliRX130nAxesVelocityController()
    {
        // make sure robot is shut down
        prepareForShutdown_cmd();
    
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
        
        if (!marshalling()->readProperties(propertyfile)) {
            log(Error) << "Failed to read the property file, continueing with default values." << endlog();
            return false;
        }  
        simulation=simulation_prop.value();

        if(!(driveLimits_prop.value().size()==STAUBLIRX130_NUM_AXES&&
             lowerPositionLimits_prop.value().size()==STAUBLIRX130_NUM_AXES&&
             upperPositionLimits_prop.value().size()==STAUBLIRX130_NUM_AXES&&
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
        return true;
    }
      
    bool StaubliRX130nAxesVelocityController::startHook()
    {
        return true;
    }
  
    void StaubliRX130nAxesVelocityController::updateHook()
    {
        driveValues_port.Get(driveValues);
                
        for (unsigned int i=0;i<STAUBLIRX130_NUM_AXES;i++) {      
#if defined OROPKG_OS_LXRT
            if (driveFailure[i]->isOn()){
                log(Error)<<"Failure drive "<<i<<", stopping all axes"<<endlog();
                prepareForShutdown();
            }
#endif
            // Set the position and perform checks in joint space.
            positionValues[i] = axes[i]->getSensor("Position")->readSensor();
            // emit event when position is out of range
            if( (positionValues[i] < lowerPositionLimits_prop.value()[i]) ||
                (positionValues[i] > upperPositionLimits_prop.value()[i]) )
                positionOutOfRange_evt("Position  of a StaubliRX130 Axis is out of range");
            // send the drive value to hw and performs checks
            if (axes[i]->isDriven())
                axes[i]->drive(driveValues[i]);

        }
        positionValues_port.Set(positionValues);
    }
    
    
    void StaubliRX130nAxesVelocityController::stopHook()
    {
        //Make sure machine is shut down
        prepareForShutdown();
    }

    void StaubliRX130nAxesVelocityController::cleanupHook()
    {
        //Write properties back to file
        marshalling()->writeProperties(propertyfile);
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
            return (eStop->isOff() && ArmPowerOn->isOn());
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
        brakeOff->switchOff()
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
                ((Axis*)(axes[i]))->getDrive()->addOffset(offset[axis]);
#endif
        }
        return true;
    }

}//namespace orocos
