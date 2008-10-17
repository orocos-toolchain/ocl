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
#include <ocl/ComponentLoader.hpp>
#include <rtt/Logger.hpp>

ORO_LIST_COMPONENT_TYPE( OCL::Kuka361nAxesTorqueController )

namespace OCL
{
    using namespace RTT;
    using namespace std;

#define KUKA361_NUM_AXES 6

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
#define KUKA361_RADproSEC2VOLT { 2.5545, 2.67804024532652, 1.37350318088664, 2.34300679603342, 2.0058, 1.7573 } //18 april 2006

// Conversion factors for tacho, unknown for last 3 axes
//#define KUKA361_TACHOSCALE { 1/9.2750, 1/10.0285, 1/4.9633, 0.113, 0, 0 }
//#define KUKA361_TACHOOFFSET { 0.0112, 0.0083, 0.0056, 0, 0, 0 }
#define KUKA361_TACHOSCALE { 0, 0, 0, 0, 0, 0 }
#define KUKA361_TACHOOFFSET { 0, 0, 0, 0, 0, 0 }

  // Conversion of current to torque: Km
// #define KUKA361_KM { 0.2781*5.77*94.14706, 0.2863*5.85*103.23529, 0.2887*5.78*51.44118, 0.07*5.7*175, 0.07*5.7*150, 0.07*5.7*131.64395 }
#define KUKA361_KM { 0.2781*94.14706, 0.2863*103.23529, 0.2887*51.44118, 0.07*175, 0.07*150, 0.07*131.64395 }


	// parameters of current regulator: I = (a*UN + b)/R + c, unknown for last 3 axes
#define KUKA361_A { 0.9030, 0.9091, 0.8929, 0.8909, 0.8962, 0.4716*1.92260 }
#define KUKA361_B { 0.0896, 0.1072, 0.0867, 0.117, 0.0822, 0.0472*1.92260 }
#define KUKA361_R { 0.1756, 0.1742, 0.1745, 0.1753, 0.1792, 0.1785 }
// #define KUKA361_C { 0,0,0,0,0,0 }
#define KUKA361_C { 0.7109, 0.3251, 0.0566, 0.1016, 0.0950, -1.0518 }


  // Channel position offset on DAQ-boards
#define TACHO_OFFSET 0
#define CURRENT_OFFSET KUKA361_NUM_AXES
#define	MODE_OFFSET 0


    typedef Kuka361nAxesTorqueController MyType;

    Kuka361nAxesTorqueController::Kuka361nAxesTorqueController(string name)
        : TaskContext(name,PreOperational),
          driveValues("nAxesOutputValue",vector<double>(KUKA361_NUM_AXES)),
          driveValues_local(KUKA361_NUM_AXES),
          positionValues("nAxesSensorPosition",vector<double>(KUKA361_NUM_AXES)),
          positionValues_local(KUKA361_NUM_AXES),
          velocityValues("nAxesSensorVelocity",vector<double>(KUKA361_NUM_AXES)),
          velocityValues_local(KUKA361_NUM_AXES),
          torqueValues("nAxesSensorTorque",vector<double>(KUKA361_NUM_AXES)),
          torqueValues_local(KUKA361_NUM_AXES),
          delta_time_Port("_delta_time_Port"),
          velocityLimits("velocityLimits","velocity limits of the axes, (rad/s)",vector<double>(KUKA361_NUM_AXES,0)),
          currentLimits("currentLimits","current limits of the axes, (A)",vector<double>(KUKA361_NUM_AXES,0)),
          lowerPositionLimits("LowerPositionLimits","Lower position limits (rad)",vector<double>(KUKA361_NUM_AXES,0)),
          upperPositionLimits("UpperPositionLimits","Upper position limits (rad)",vector<double>(KUKA361_NUM_AXES,0)),
          initialPosition("initialPosition","Initial position (rad) for simulation or hardware",vector<double>(KUKA361_NUM_AXES,0)),
          velDriveOffset("velDriveOffset","offset (in rad/s) to the drive value.",vector<double>(KUKA361_NUM_AXES,0)),
          simulation("simulation","true if simulationAxes should be used",true),
          num_axes("NUM_AXES",KUKA361_NUM_AXES),
          velocityOutOfRange("velocityOutOfRange"),
          currentOutOfRange("currentOutOfRange"),
          positionOutOfRange("positionOutOfRange"),
          velresolved("velresolved","true if robot should be velocityresolved",false),
          TorqueMode(false),
          activated(false),
          positionConvertFactor(KUKA361_NUM_AXES),
          driveConvertFactor(KUKA361_NUM_AXES),
          tachoConvertScale(KUKA361_NUM_AXES),
          tachoConvertOffset(KUKA361_NUM_AXES),
          curReg_a(KUKA361_NUM_AXES),
          curReg_b(KUKA361_NUM_AXES),
          shunt_R(KUKA361_NUM_AXES),
          shunt_c(KUKA361_NUM_AXES),
          Km(KUKA361_NUM_AXES),
          EmergencyEvents_prop("EmergencyEvents","List of events that will result in an emergencystop of the robot"),
#if (defined (OROPKG_OS_LXRT))
          axes_hardware(KUKA361_NUM_AXES),
          encoderInterface(KUKA361_NUM_AXES),
          encoder(KUKA361_NUM_AXES),
          ref(KUKA361_NUM_AXES),
          enable(KUKA361_NUM_AXES),
          drive(KUKA361_NUM_AXES),
          brake(KUKA361_NUM_AXES),
          tachoInput(KUKA361_NUM_AXES),
          tachometer(KUKA361_NUM_AXES),
          currentInput(KUKA361_NUM_AXES),
          currentSensor(KUKA361_NUM_AXES),
          torqueModeCheck(KUKA361_NUM_AXES),
#endif
          axes(KUKA361_NUM_AXES),
          axes_simulation(KUKA361_NUM_AXES),
          tau_sim(KUKA361_NUM_AXES,0.0),
          pos_sim(KUKA361_NUM_AXES),
          previous_time(KUKA361_NUM_AXES)
    {
        double ticks2rad[KUKA361_NUM_AXES] = KUKA361_TICKS2RAD;
        double vel2volt[KUKA361_NUM_AXES] = KUKA361_RADproSEC2VOLT;
        double tachoscale[KUKA361_NUM_AXES] = KUKA361_TACHOSCALE;
        double tachooffset[KUKA361_NUM_AXES] = KUKA361_TACHOOFFSET;
        double CurReg_a[KUKA361_NUM_AXES] = KUKA361_A;
        double CurReg_b[KUKA361_NUM_AXES] = KUKA361_B;
        double Shunt_R[KUKA361_NUM_AXES] = KUKA361_R;
        double Shunt_c[KUKA361_NUM_AXES] = KUKA361_C;
        double KM[KUKA361_NUM_AXES] = KUKA361_KM;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            positionConvertFactor[i] = ticks2rad[i];
            driveConvertFactor[i] = vel2volt[i];
            tachoConvertScale[i] = tachoscale[i];
            tachoConvertOffset[i] = tachooffset[i];
            curReg_a[i] = CurReg_a[i];
            curReg_b[i] = CurReg_b[i];
            shunt_R[i] = Shunt_R[i];
            shunt_c[i] = Shunt_c[i];
            Km[i] = KM[i];
        }

        // make task context
        properties()->addProperty( &velocityLimits );
        properties()->addProperty( &currentLimits );
        properties()->addProperty( &lowerPositionLimits );
        properties()->addProperty( &upperPositionLimits  );
        properties()->addProperty( &initialPosition  );
        properties()->addProperty( &velDriveOffset  );
        properties()->addProperty( &simulation  );
        properties()->addProperty( &velresolved);
        properties()->addProperty( &EmergencyEvents_prop);
        attributes()->addConstant( &num_axes);

        
        this->methods()->addMethod( method("startAxes", &MyType::startAllAxes, this),
                                    "start all axes, starts updating the drive-value (only possible after unlockAxes)");
        this->methods()->addMethod( method("stopAxes", &MyType::stopAllAxes, this),
                                    "stop all axes, sets drive value to zero and disables the update of the drive-port, (only possible if axis is started)");
        this->methods()->addMethod( method("lockAxes", &MyType::lockAllAxes, this),"lock all axes, enables the brakes (only possible if axis is stopped");
        this->methods()->addMethod( method( "unlockAxes", &MyType::unlockAllAxes, this),
                                    "unlock all axis, disables the brakes and enables the drives (only possible if axis is locked");
        this->commands()->addCommand(command("prepareForUse", &MyType::prepareForUse,&MyType::prepareForUseCompleted, this),
                                     "prepares the robot for use"  );
        this->commands()->addCommand(command( "prepareForShutdown", &MyType::prepareForShutdown,&MyType::prepareForShutdownCompleted, this),
                                     "prepares the robot for shutdown"  );
        this->methods()->addMethod( method("addDriveOffset", &MyType::addDriveOffset, this),
                                    "adds offsets to the drive value of the axes","offsets","offset values in rad/s" );
        
        /**
         * Creating and adding the data-ports
         */
        this->ports()->addPort(&driveValues);
        this->ports()->addPort(&positionValues);
        this->ports()->addPort(&velocityValues);
        this->ports()->addPort(&torqueValues);
        this->ports()->addPort(&delta_time_Port);
        
        /**
         * Adding the events :
         */
        events()->addEvent( &velocityOutOfRange, "Velocity of an Axis is out of range","message","Information about event" );
        events()->addEvent( &positionOutOfRange, "Position of an Axis is out of range","message","Information about event");
        events()->addEvent( &currentOutOfRange, "Current of an Axis is out of range","message","Information about event");

        
    }
    
    bool Kuka361nAxesTorqueController::configureHook()
    {
            
        TorqueMode = !velresolved.rvalue();
	

#if (defined (OROPKG_OS_LXRT))
        int encoderOffsets[KUKA361_NUM_AXES] = KUKA361_ENCODEROFFSETS;
        
        comediDev        = new ComediDevice( 1 );
        comediSubdevAOut = new ComediSubDeviceAOut( comediDev, "NI6713_AO" );
        apci1710         = new EncoderSSI_apci1710_board( 0, 1, 2 );
        apci2200         = new RelayCardapci2200( "APCI2200" );
        apci1032         = new SwitchDigitalInapci1032( "APCI1032" );
        comediDev_NI6024  = new ComediDevice( 4 );
        comediSubdevAIn_NI6024  = new ComediSubDeviceAIn( comediDev_NI6024, "NI6024_AI", 0 );
        comediSubdevDIn_NI6024  = new ComediSubDeviceDIn( comediDev_NI6024, "NI6024_DI", 2 );
        comediSubdevDOut_NI6713  = new ComediSubDeviceDOut( comediDev, "NI6713_AO", 2 );
        
        torqueModeSwitch = new DigitalOutput(comediSubdevDOut_NI6713,0);
        if(TorqueMode)
            torqueModeSwitch->switchOn();
        else
            torqueModeSwitch->switchOff();
        
        //    _encoderInterface[i] = new EncoderSSI_apci1710( i + 1, _apci1710 );
        //Setting up encoderinterfaces:
        encoderInterface[0] = new EncoderSSI_apci1710( 1, apci1710 );
        encoderInterface[1] = new EncoderSSI_apci1710( 2, apci1710 );
        encoderInterface[2] = new EncoderSSI_apci1710( 7, apci1710 );
        encoderInterface[3] = new EncoderSSI_apci1710( 8, apci1710 );
        encoderInterface[4] = new EncoderSSI_apci1710( 5, apci1710 );
        encoderInterface[5] = new EncoderSSI_apci1710( 6, apci1710 );
        
        for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++){
            //Setting up encoders
            encoder[i] = new AbsoluteEncoderSensor( encoderInterface[i], 1.0 / positionConvertFactor[i], encoderOffsets[i], -10, 10 );
            
            brake[i] = new DigitalOutput( apci2200, i + KUKA361_NUM_AXES );
            log(Info)<<"Setting brake "<<i<<" On."<<endlog();
            brake[i]->switchOn();
            
            tachoInput[i] = new AnalogInput(comediSubdevAIn_NI6024, i+TACHO_OFFSET);
            unsigned int range = 0; // The input range is -10 to 10 V, so range 0
            comediSubdevAIn_NI6024->rangeSet(i+TACHO_OFFSET, range);
            comediSubdevAIn_NI6024->arefSet(i+TACHO_OFFSET, AnalogInInterface::Common);
            tachometer[i] = new AnalogSensor( tachoInput[i], comediSubdevAIn_NI6024->lowest(i+TACHO_OFFSET), comediSubdevAIn_NI6024->highest(i+TACHO_OFFSET), tachoConvertScale[i], tachoConvertOffset[i]);
            
            currentInput[i] = new AnalogInput(comediSubdevAIn_NI6024, i+CURRENT_OFFSET);
            range = 1; // for a input range -5 to 5 V, range is 1
            comediSubdevAIn_NI6024->rangeSet(i+CURRENT_OFFSET, range);
            comediSubdevAIn_NI6024->arefSet(i+CURRENT_OFFSET, AnalogInInterface::Common);
            currentSensor[i] = new AnalogSensor( currentInput[i], comediSubdevAIn_NI6024->lowest(i+CURRENT_OFFSET), comediSubdevAIn_NI6024->highest(i+CURRENT_OFFSET), 1.0 , 0); // 1.0 / _shunt_R[i]
            
            torqueModeCheck[i] = new DigitalInput( comediSubdevDIn_NI6024, i );
            
            ref[i]   = new AnalogOutput( comediSubdevAOut, i );
            enable[i] = new DigitalOutput( apci2200, i );

            
            if ( TorqueMode ){
                if ( !torqueModeCheck[i]->isOn() ) {
                    log(Error) << "Failed to switch relay of channel " << i << " to torque control mode" << endlog();
                    return false;
                }
                drive[i]  = new AnalogDrive( ref[i], enable[i], curReg_a[i] / shunt_R[i], - shunt_c[i] - (curReg_b[i] / shunt_R[i]));
                
            }else{
                drive[i]  = new AnalogDrive( ref[i], enable[i], 1.0 / driveConvertFactor[i], velDriveOffset.value()[i]);
            }
            
            axes_hardware[i] = new Axis( drive[i] );
            axes_hardware[i]->setBrake( brake[i] );
            axes_hardware[i]->setSensor( "Position", encoder[i] );
            axes_hardware[i]->setSensor( "Velocity", tachometer[i] );
            axes_hardware[i]->setSensor( "Current", currentSensor[i] );
            axes_hardware[i]->setSwitch( "Mode", torqueModeCheck[i] );
            
            if ( TorqueMode ){
                axes_hardware[i]->limitDrive(-currentLimits.value()[i], currentLimits.value()[i], currentOutOfRange);
            }else{
                axes_hardware[i]->limitDrive(-velocityLimits.value()[i], velocityLimits.value()[i], velocityOutOfRange);
            }
        }
        
        
#endif
        for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++){
            if ( TorqueMode ) {
                axes_simulation[i] = new OCL::TorqueSimulationAxis(initialPosition.value()[i], lowerPositionLimits.value()[i], upperPositionLimits.value()[i],velocityLimits.value()[i]);
            } else {
                axes_simulation[i] = new OCL::SimulationAxis(initialPosition.value()[i], lowerPositionLimits.value()[i], upperPositionLimits.value()[i]);
            }
        }
        
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            pos_sim[i] = initialPosition.value()[i];
        }
        torqueSimulator = new Kuka361TorqueSimulator(axes_simulation, pos_sim);
        
#if (defined (OROPKG_OS_LXRT))
        if(!simulation.rvalue()){
            for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++)
                axes[i] = axes_hardware[i];
            log(Info) << "LXRT version of Kuka361nAxesTorqueController has started" << endlog();
        }
        else{
            for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++)
                axes[i] = axes_simulation[i];
            log(Info) << "LXRT simulation version of Kuka361nAxesTorqueController has started" << endlog();
        }
#else
        for (unsigned int i = 0; i <KUKA361_NUM_AXES; i++)
            axes[i] = axes_simulation[i];
        log(Info) << "GNULINUX simulation version of Kuka361nAxesTorqueController has started" << endlog();
#endif
        if(EmergencyEvents_prop.ready())
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
                    Handle handle = peer->events()->setupConnection(eventname).callback(this,&Kuka361nAxesTorqueController::EmergencyStop).handle();
                    if(handle.connect()){
                        EmergencyEventHandlers.push_back(handle);
                        log(Info)<<"EmergencyStop connected to "<< name<<" event."<<endlog();
                    }else
                        log(Warning)<<"Could not connect EmergencyStop to "<<name<<", "<<eventname<<" has to have a message parameter."<<endlog();
                }else
                    log(Warning)<<"Could not connect EmergencyStop to "<<name<<", "<<eventname <<" not found in "<<peername<<"s event-list"<<endlog();
            }else{
            log(Warning)<<"No EmergencyStops available"<<endlog();
        }
        
        return true;
    }
    
    
    Kuka361nAxesTorqueController::~Kuka361nAxesTorqueController()
    {
    }
    
    void Kuka361nAxesTorqueController::cleanupHook()
    {
        // make sure robot is shut down
        prepareForShutdown();

        // brake, drive, sensors and switches are deleted by each axis
        for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++)
            delete axes_simulation[i];
        
        delete torqueSimulator;

#if (defined (OROPKG_OS_LXRT))
        for (unsigned int i = 0; i < KUKA361_NUM_AXES; i++)
            delete axes_hardware[i];
        delete comediDev;
        delete comediSubdevAOut;
        delete apci1710;
        delete apci2200;
        delete apci1032;
        delete comediDev_NI6024;
        delete comediSubdevAIn_NI6024;
        delete comediSubdevDIn_NI6024;

#endif
    }
    

    bool Kuka361nAxesTorqueController::startHook()
    {
        for (int axis=0;axis<KUKA361_NUM_AXES;axis++) {
            previous_time[axis] = TimeService::Instance()->getTicks();
            positionValues_local[axis] = axes[axis]->getSensor("Position")->readSensor();
        }
        positionValues.Set(positionValues_local);
        
        return true;
    }
    
    void Kuka361nAxesTorqueController::updateHook()
    {
        driveValues_local = driveValues.Get();
        positionValues_local = positionValues.Get();
#if (defined (OROPKG_OS_LXRT))
        if(simulation.rvalue()) {
            //simulate kuka 361 in torque mode
            
            torqueSimulator->update(driveValues_local);
        }
#else
        //simulate kuka 361
        torqueSimulator->update(driveValues_local);
#endif

        for (int axis=0;axis<KUKA361_NUM_AXES;axis++) {
            // Set the velocity and the position and perform checks in joint space.
            delta_time = TimeService::Instance()->secondsSince(previous_time[axis]);
            velocityValues_local[axis] = (axes[axis]->getSensor("Position")->readSensor() - positionValues_local[axis])/delta_time;
            previous_time[axis] = TimeService::Instance()->getTicks();
            positionValues_local[axis] = axes[axis]->getSensor("Position")->readSensor();
            delta_time_Port.Set(delta_time);

            // emit event when velocity is out of range
            if( (velocityValues_local[axis] < -velocityLimits.value()[axis]) ||
                (velocityValues_local[axis] > velocityLimits.value()[axis]) ){
                char msg[80];
                sprintf(msg,"Velocity of Kuka361 Axis %d is out of range: %f",axis+1,velocityValues_local[axis]);
                velocityOutOfRange(msg);
            }
            
            // emit event when position is out of  range
            if( (positionValues_local[axis] < lowerPositionLimits.value()[axis]) ||
                (positionValues_local[axis] > upperPositionLimits.value()[axis]) ){
                char msg[80];
                sprintf(msg,"Position of Kuka361 Axis %d is out of range: %f",axis+1,positionValues_local[axis]);
                positionOutOfRange(msg);
            }

            // send the drive value to hw and performs checks, convert torque to current if torque controlled
            if (axes[axis]->isDriven()) {
                if( TorqueMode ){
                    axes[axis]->drive(driveValues_local[axis] / Km[axis]); // accepts a current
                }
                else
                    axes[axis]->drive(driveValues_local[axis]);
            }
            
#if (defined (OROPKG_OS_LXRT))
            if(!simulation.rvalue()) {
                // Set the measured current
                if( TorqueMode){
                    torqueValues_local[axis] = (axes[axis]->getSensor("Current")->readSensor() / shunt_R[axis] + shunt_c[axis]) * Km[axis];
                }
            }
#endif
        }
        torqueValues.Set(torqueValues_local);
        velocityValues.Set(velocityValues_local);
        positionValues.Set(positionValues_local);
    }
  
	    

    void Kuka361nAxesTorqueController::stopHook()
    {
        //Make sure machine is shut down
        prepareForShutdown();
        //Write properties back to file
        //marshalling()->writeProperties(_propertyfile);
    }


    bool Kuka361nAxesTorqueController::prepareForUse()
    {
#if (defined (OROPKG_OS_LXRT))
        if(!simulation.rvalue()){
            apci2200->switchOn( 12 );
            apci2200->switchOn( 14 );
            log(Warning) <<"Release Emergency stop and push button to start ..."<<endlog();
        }
#endif
        activated = true;
        return true;
    }

    bool Kuka361nAxesTorqueController::prepareForUseCompleted()const
    {
#if (defined (OROPKG_OS_LXRT))
        if(!simulation.rvalue())
            return (apci1032->isOn(12) && apci1032->isOn(14));
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
        torqueModeSwitch->switchOff();
        
        if(!simulation.rvalue()){
            apci2200->switchOff( 12 );
            apci2200->switchOff( 14 );
        }

#endif
        activated = false;
        return true;
    }

    bool Kuka361nAxesTorqueController::prepareForShutdownCompleted()const
    {
        return true;
    }

    bool Kuka361nAxesTorqueController::stopAllAxes()
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            _return &= axes[i]->stop();
        }
        return _return;
    }

    bool Kuka361nAxesTorqueController::startAllAxes()
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            _return &= axes[i]->drive(0.0);
        }
        return _return;
    }

    bool Kuka361nAxesTorqueController::unlockAllAxes()
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            _return &= axes[i]->unlock();
        }
        return _return;
    }

    bool Kuka361nAxesTorqueController::lockAllAxes()
    {
        bool _return = true;
        for(unsigned int i = 0;i<KUKA361_NUM_AXES;i++){
            _return &= axes[i]->lock();
        }
        return _return;
    }


    bool Kuka361nAxesTorqueController::addDriveOffset(const vector<double>& offset)
    {
        for(unsigned int i=0;i<KUKA361_NUM_AXES;i++){
            velDriveOffset.value()[i] += offset[i];
            
#if (defined (OROPKG_OS_LXRT))
            if (!simulation.rvalue())
                ((Axis*)(axes[i]))->getDrive()->addOffset(offset[i]);
#endif
        }
        return true;
    }

}//namespace orocos

