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
#include <ocl/ComponentLoader.hpp>

#include <rtt/Logger.hpp>
#include <stdlib.h>

namespace OCL
{
    using namespace RTT;
    using namespace KDL;
    using namespace std;
  
#define KUKA361_NUM_AXES 6

//#define KUKA361_ENCODEROFFSETS { 1000004, 1000000, 1000002, 449784,
//#1035056, 1230656 }
//New values for axis 3 and 4 12/12/2007
#define KUKA361_ENCODEROFFSETS { 1000004, 1000000, 502652, 1001598, 985928, 1230656 }

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
//#define KUKA361_RADproSEC2VOLT { 2.5545, 2.67804024532652, 1.37350318088664, 2.34300679603342, 2.0058, 3.3786 } //18 april 2006
#define KUKA361_RADproSEC2VOLT { 2.5545, 2.67804024532652, 1.37350318088664, 2.34300679603342, 2.0058, 1.7573 } //24 april 2007

#define KINEMATICS_EPS 0.0001
#define SQRT3d2 0.8660254037844386
#define M_PI_T2 2 * M_PI
#define SQRT3t2 3.46410161513775 // 2 sqrt(3)
    
    typedef Kuka361nAxesVelocityController MyType;
    
    Kuka361nAxesVelocityController::Kuka361nAxesVelocityController(string name)
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
          driveLimits_prop("driveLimits","velocity limits of the axes, (rad/s)",vector<double>(KUKA361_NUM_AXES,0)),
          lowerPositionLimits_prop("LowerPositionLimits","Lower position limits (rad)",vector<double>(KUKA361_NUM_AXES,0)),
          upperPositionLimits_prop("UpperPositionLimits","Upper position limits (rad)",vector<double>(KUKA361_NUM_AXES,0)),
          initialPosition_prop("initialPosition","Initial position (rad) for simulation or hardware",vector<double>(KUKA361_NUM_AXES,0)),
          driveOffset_prop("driveOffset","offset (in rad/s) to the drive value.",vector<double>(KUKA361_NUM_AXES,0)),
          simulation_prop("simulation","true if simulationAxes should be used",true),
          simulation(true),
          geometric_prop("geometric","true if drive and positions should be converted for kinematic use",true),
          EmergencyEvents_prop("EmergencyEvents","List of events that will result in an emergencystop of the robot"),
          num_axes_attr("NUM_AXES",KUKA361_NUM_AXES),
          chain_attr("Kinematics"),
          driveOutOfRange_evt("driveOutOfRange"),
          positionOutOfRange_evt("positionOutOfRange"),
          activated(false),
          positionConvertFactor(KUKA361_NUM_AXES),
          driveConvertFactor(KUKA361_NUM_AXES),
          positionValues(KUKA361_NUM_AXES),
          driveValues(KUKA361_NUM_AXES),
          positionValues_kin(KUKA361_NUM_AXES),
          driveValues_rob(KUKA361_NUM_AXES),
#if (defined OROPKG_OS_LXRT)
          axes_hardware(KUKA361_NUM_AXES),
          encoderInterface(KUKA361_NUM_AXES),
          encoder(KUKA361_NUM_AXES),
          vref(KUKA361_NUM_AXES),
          enable(KUKA361_NUM_AXES),
          drive(KUKA361_NUM_AXES),
          brake(KUKA361_NUM_AXES),
#endif
          axes(KUKA361_NUM_AXES)
    {
        Logger::In in(this->getName().data());
        double ticks2rad[KUKA361_NUM_AXES] = KUKA361_TICKS2RAD;
        double vel2volt[KUKA361_NUM_AXES] = KUKA361_RADproSEC2VOLT;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            positionConvertFactor[i] = ticks2rad[i];
            driveConvertFactor[i] = vel2volt[i];
        }

#if (defined OROPKG_OS_LXRT)
        int encoderOffsets[KUKA361_NUM_AXES] = KUKA361_ENCODEROFFSETS;
        
        log(Info)<<"Creating Comedi Devices."<<endlog();
        comediDev        = new ComediDevice( 1 );
        comediSubdevAOut = new ComediSubDeviceAOut( comediDev, "Kuka361" );
        log(Info)<<"Creating APCI Devices."<<endlog();
        apci1710         = new EncoderSSI_apci1710_board( 0, 1 , 2);
        apci2200         = new RelayCardapci2200( "Kuka361" );
        apci1032         = new SwitchDigitalInapci1032( "Kuka361" );
        
        
        //Setting up encoderinterfaces:
        encoderInterface[0] = new EncoderSSI_apci1710( 1, apci1710 );
        encoderInterface[1] = new EncoderSSI_apci1710( 2, apci1710 );
        encoderInterface[2] = new EncoderSSI_apci1710( 7, apci1710 );
        encoderInterface[3] = new EncoderSSI_apci1710( 8, apci1710 );
        encoderInterface[4] = new EncoderSSI_apci1710( 5, apci1710 );
        encoderInterface[5] = new EncoderSSI_apci1710( 6, apci1710 );
	
	
        for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++){
            log(Info)<<"Creating Hardware axis "<<i<<endlog();
            //Setting up encoders
            log(Info)<<"Setting up encoder ..."<<endlog();
            encoder[i]          = new AbsoluteEncoderSensor( encoderInterface[i], 1.0 / ticks2rad[i], encoderOffsets[i], -10, 10 );
            
            log(Info)<<"Setting up brake ..."<<endlog();
            brake[i] = new DigitalOutput( apci2200, i + KUKA361_NUM_AXES );
            log(Info)<<"Setting brake to on"<<endlog();
            brake[i]->switchOn();
            
            log(Info)<<"Setting up drive ..."<<endlog();
            vref[i]   = new AnalogOutput( comediSubdevAOut, i );
            enable[i] = new DigitalOutput( apci2200, i );
            drive[i]  = new AnalogDrive( vref[i], enable[i], 1.0 / vel2volt[i], 0.0);
            
            axes_hardware[i] = new Axis( drive[i] );
            axes_hardware[i]->setBrake( brake[i] );
            axes_hardware[i]->setSensor( "Position", encoder[i] );
        }
        
#endif
        //Definition of kinematics for the Kuka361 
        kinematics.addSegment(Segment(Joint(Joint::RotZ),Frame(Vector(0.0,0.0,1.020))));
        kinematics.addSegment(Segment(Joint(Joint::RotX),Frame(Vector(0.0,0.0,0.480))));
        kinematics.addSegment(Segment(Joint(Joint::RotX),Frame(Vector(0.0,0.0,0.645))));
        kinematics.addSegment(Segment(Joint(Joint::RotZ)));
        kinematics.addSegment(Segment(Joint(Joint::RotX)));
        kinematics.addSegment(Segment(Joint(Joint::RotZ),Frame(Vector(0.0,0.0,0.120))));
        
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
        properties()->addProperty( &geometric_prop);
        properties()->addProperty( &EmergencyEvents_prop);
        attributes()->addConstant( &num_axes_attr);
        attributes()->addAttribute(&chain_attr);

    }
    
    Kuka361nAxesVelocityController::~Kuka361nAxesVelocityController()
    {
        // make sure robot is shut down
        prepareForShutdown_cmd();
    
        // brake, drive, sensors and switches are deleted by each axis
        if(simulation)
            for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++)
                delete axes[i];
    
#if (defined OROPKG_OS_LXRT)
        for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++)
            delete axes_hardware[i];
        delete comediDev;
        delete comediSubdevAOut;
        delete apci1710;
        delete apci2200;
        delete apci1032;
#endif
    }
  
    bool Kuka361nAxesVelocityController::configureHook()
    {
        Logger::In in(this->getName().data());
        
        simulation=simulation_prop.value();

        if(!(driveLimits_prop.value().size()==KUKA361_NUM_AXES&&
             lowerPositionLimits_prop.value().size()==KUKA361_NUM_AXES&&
             upperPositionLimits_prop.value().size()==KUKA361_NUM_AXES&&
             initialPosition_prop.value().size()==KUKA361_NUM_AXES&&
             driveOffset_prop.value().size()==KUKA361_NUM_AXES))
            return false;
        
#if (defined OROPKG_OS_LXRT)
        if(!simulation){
            for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++){
	      axes_hardware[i]->limitDrive(-driveLimits_prop.value()[i], driveLimits_prop.value()[i], driveOutOfRange_evt);
	      axes[i] = axes_hardware[i];
	      ((Axis*)(axes[i]))->getDrive()->addOffset(driveOffset_prop.value()[i]);
            }
	    log(Info) << "Hardware version of Kuka361nAxesVelocityController has started" << endlog();
        }
        else{
#endif
            for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++)
                axes[i] = new SimulationAxis(initialPosition_prop.value()[i],
                                             lowerPositionLimits_prop.value()[i],
                                             upperPositionLimits_prop.value()[i]);
            log(Info) << "Simulation version of Kuka361nAxesVelocityController has started" << endlog();
#if (defined OROPKG_OS_LXRT)
        }
#endif
        for(unsigned int i=0;i<EmergencyEvents_prop.value().size();i++){
            string name = EmergencyEvents_prop.value()[i];
            string::size_type idx = name.find('.');
            if(idx==string::npos)
                log(Warning)<<"Could not connect EmergencyStop to "<<name<<"\n Syntax of "
                          <<name<<" is not correct. I want a ComponentName.EventName "<<endlog();
            string peername = name.substr(0,idx);
            string eventname = name.substr(idx+1);
            TaskContext* peer;
            if(peername==this->getName())
                peer = this;
            else if(this->hasPeer(peername)){
                peer = this->getPeer(peername);
            }else{
                log(Warning)<<"Could not connect EmergencyStop to "<<name<<", "<<peername<<" is not a peer of "<<this->getName()<<endlog();
                continue;
            }
            
            if(peer->events()->hasEvent(eventname)){
                Handle handle = peer->events()->setupConnection(eventname).callback(this,&Kuka361nAxesVelocityController::EmergencyStop).handle();
                if(handle.connect()){
                    EmergencyEventHandlers.push_back(handle);
                    log(Info)<<"EmergencyStop connected to "<< name<<" event."<<endlog();
                }else
                    log(Warning)<<"Could not connect EmergencyStop to "<<name<<", "<<eventname<<" has to have a message parameter."<<endlog();
            }else
                log(Warning)<<"Could not connect EmergencyStop to "<<name<<", "<<eventname <<" not found in "<<peername<<"s event-list"<<endlog();
        }
        return true;
    }
      
    bool Kuka361nAxesVelocityController::startHook()
    {
        return true;
    }
  
    void Kuka361nAxesVelocityController::updateHook()
    {
        for (unsigned int axis=0;axis<KUKA361_NUM_AXES;axis++) {      
            // Set the position and perform checks in joint space.
            positionValues[axis] = axes[axis]->getSensor("Position")->readSensor();
        }
        
        driveValues_port.Get(driveValues);

        if(geometric_prop.value())
            convertGeometric();
        
        positionValues_port.Set(positionValues);
        
        for (unsigned int axis=0;axis<KUKA361_NUM_AXES;axis++){
            if (axes[axis]->isDriven())
                axes[axis]->drive(driveValues[axis]);
	    
            // emit event when position is out of range
            if( (positionValues[axis] < lowerPositionLimits_prop.value()[axis]) ||
                (positionValues[axis] > upperPositionLimits_prop.value()[axis]) )
                positionOutOfRange_evt("Position  of a Kuka361 Axis is out of range");
            
            // send the drive value to hw and performs checks
        }
    }
    
    
    void Kuka361nAxesVelocityController::stopHook()
    {
        //Make sure machine is shut down
        prepareForShutdown();
    }

    void Kuka361nAxesVelocityController::cleanupHook()
    {
    }
        
    bool Kuka361nAxesVelocityController::prepareForUse()
    {
#if (defined OROPKG_OS_LXRT)
        if(!simulation){
            apci2200->switchOn( 12 );
            apci2200->switchOn( 14 );
            log(Warning) <<"Release Emergency stop and push button to start ...."<<endlog();
        }
#endif
        activated = true;
        return true;
    }
    
    bool Kuka361nAxesVelocityController::prepareForUseCompleted()const
    {
#if (defined OROPKG_OS_LXRT)
        if(!simulation)
            return (apci1032->isOn(12) && apci1032->isOn(14));
        else
#endif
            return true;
    }
    
    bool Kuka361nAxesVelocityController::prepareForShutdown()
    {
        //make sure all axes are stopped and locked
        stopAllAxes();
        lockAllAxes();
#if (defined OROPKG_OS_LXRT)
        if(!simulation){
            apci2200->switchOff( 12 );
            apci2200->switchOff( 14 );
        }
        
#endif
        activated = false;
        return true;
    }
    
    bool Kuka361nAxesVelocityController::prepareForShutdownCompleted()const
    {
        return true;
    }
    
    bool Kuka361nAxesVelocityController::stopAllAxes()
    {
        bool succes = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++)
            succes &= axes[i]->stop();
        
        return succes;
    }
  
    bool Kuka361nAxesVelocityController::startAllAxes()
    {
        bool succes = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++)
            succes &= axes[i]->drive(0.0);
        
        return succes;
    }
  
    bool Kuka361nAxesVelocityController::unlockAllAxes()
    {
        if(!activated)
            return false;
        
        bool succes = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++)
            succes &= axes[i]->unlock();

        return succes;
    }
  
    bool Kuka361nAxesVelocityController::lockAllAxes()
    {
        bool succes = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            succes &= axes[i]->lock();
        }
        return succes;
    }
    
    bool Kuka361nAxesVelocityController::addDriveOffset(const vector<double>& offset)
    {
        if(offset.size()!=KUKA361_NUM_AXES)
            return false;
        
        for(unsigned int i=0;i<KUKA361_NUM_AXES;i++){
        if(geometric_prop.value() && (i==0 || i==3 || i==5) )
        {
            driveOffset_prop.value()[i] -= offset[i];
        }
        else
        {
            driveOffset_prop.value()[i] += offset[i];
        }
#if (defined OROPKG_OS_LXRT)
            if (!simulation)
            {
                if(geometric_prop.value() && (i==0 || i==3 || i==5) )
	              ((Axis*)(axes[i]))->getDrive()->addOffset(-offset[i]);
                else
	              ((Axis*)(axes[i]))->getDrive()->addOffset(offset[i]);
            } 
#endif
        }
        return true;
    }

    void Kuka361nAxesVelocityController::convertGeometric()
    {
        positionValues[0] = -positionValues[0];
        positionValues[3] = -positionValues[3];
        positionValues[5] = -positionValues[5];
        driveValues[0] = -driveValues[0];
        driveValues[3] = -driveValues[3];
        driveValues[5] = -driveValues[5];
        for(unsigned int i = 0; i < 3 ; i++){
            positionValues_kin[i] = positionValues[i];
            driveValues_rob[i] = driveValues[i];
        }
        
        // convert last 3 axis from DWH into ZXZ
        double c5 = cos(positionValues[4]);
        double s5 = sin(positionValues[4]);
        double c5_eq = (c5+3.)/4;   /* eq.(3-1) inverse */
        double alpha;
        
        if (positionValues[4]<-KINEMATICS_EPS){
            alpha = atan2(-s5,SQRT3d2*(c5-1.));  /* eq.(3-3)/(3-4) */
            positionValues_kin[4]=-2.*acos(c5_eq);
            driveValues_rob[ 4 ]=-sqrt((1.-c5)*(7.+c5))*driveValues[4]/2./s5;
        }else{
            if (positionValues[4]<KINEMATICS_EPS){
                alpha = M_PI_2;
                positionValues_kin[4] = 0.0;
                driveValues_rob[4]=driveValues[4];
            }else{
                alpha = atan2( s5, SQRT3d2 * ( 1. - c5 ) );
                positionValues_kin[4] = 2.*acos(c5_eq);
                driveValues_rob[4] = sqrt((1.-c5)*(7.+c5))*driveValues[4]/2./s5;
            }
        }
        
        positionValues_kin[ 3 ] = positionValues[ 3 ] + alpha;
        positionValues_kin[ 5 ] = positionValues[ 5 ] - alpha;
        
        double alphadot = -SQRT3t2/(7.+c5)*driveValues[4];
            
        driveValues_rob[3] = driveValues[3]-alphadot;
        driveValues_rob[5] = driveValues[5]+alphadot;
        
        driveValues.swap(driveValues_rob);
        positionValues.swap(positionValues_kin);
        
    }
    
      
}//namespace orocos
ORO_CREATE_COMPONENT_TYPE()
ORO_LIST_COMPONENT_TYPE( OCL::Kuka361nAxesVelocityController )
