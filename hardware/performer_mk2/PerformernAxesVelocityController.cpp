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

#include "PerformernAxesVelocityController.hpp"

#include <ocl/ComponentLoader.hpp>

#include <rtt/Logger.hpp>
#include <sstream>

namespace OCL
{
using namespace RTT;
using namespace KDL;
using namespace std;

#define PERFORMERMK2_NUM_AXES 5

#define PERFORMERMK2_ENCODEROFFSETS { 0, 0, 0, 0, 0 }

//Encoder counts for pi/2 radians
#define PERFORMERMK2_CONV1  60000
#define PERFORMERMK2_CONV2  80000
#define PERFORMERMK2_CONV3  72000
#define PERFORMERMK2_CONV4  60000
#define PERFORMERMK2_CONV5  44000

// Conversion from encoder ticks to radiants
#define PERFORMERMK2_TICKS2RAD { M_PI_2 / PERFORMERMK2_CONV1 ,M_PI_2 / PERFORMERMK2_CONV2 ,M_PI_2 / PERFORMERMK2_CONV3 ,M_PI_2 / PERFORMERMK2_CONV4 ,M_PI_2 / PERFORMERMK2_CONV5 }

// Conversion from angular speed to voltage
#define PERFORMERMK2_RADproSEC2VOLT { 5.25, 12.84, 12.22, 4.4, 3.1}

// Velocity estimator parameters

// maximum number of backward steps
# define m 10
// relative accuracy
# define rj 0.02
# define sj 2/rj
# define pi 3.14159265359
# define R1 pi/2/PERFORMERMK2_CONV1
# define R2 pi/2/PERFORMERMK2_CONV2
# define R3 pi/2/PERFORMERMK2_CONV3
# define R4 pi/2/PERFORMERMK2_CONV4
# define R5 pi/2/PERFORMERMK2_CONV5

typedef PerformerMK2nAxesVelocityController MyType;

PerformerMK2nAxesVelocityController::PerformerMK2nAxesVelocityController(string name,string _propertyfile)
: TaskContext(name,PreOperational),
resetController_mtd( "resetController", &MyType::resetController, this),
startAllAxes_mtd( "startAllAxes", &MyType::startAllAxes, this),
stopAllAxes_mtd( "stopAllAxes", &MyType::stopAllAxes, this),
unlockAllAxes_mtd( "unlockAllAxes", &MyType::unlockAllAxes, this),
lockAllAxes_mtd( "lockAllAxes", &MyType::lockAllAxes, this),
prepareForUse_cmd( "prepareForUse", &MyType::prepareForUse,&MyType::prepareForUseCompleted, this),
prepareForShutdown_cmd( "prepareForShutdown", &MyType::prepareForShutdown,&MyType::prepareForShutdownCompleted, this),
addDriveOffset_mtd( "addDriveOffset", &MyType::addDriveOffset, this),
_initPosition( "initPosition", &MyType::initPosition, this),
driveValues_port("nAxesOutputVelocity"),
servoValues_port("nAxesServoVelocity"),
positionValues_port("nAxesSensorPosition"),
velocityValues_port("nAxesSensorVelocity"),
jValues_port("nAxesjValues"),
homingSwitchValues_port("nAxesSensorHomingSwitch"),
deltaTime_port("DeltaTime"),
driveLimits_prop("driveLimits","velocity limits of the axes, (rad/s)",vector<double>(PERFORMERMK2_NUM_AXES,0)),
lowerPositionLimits_prop("LowerPositionLimits","Lower position limits (rad)",vector<double>(PERFORMERMK2_NUM_AXES,0)),
upperPositionLimits_prop("UpperPositionLimits","Upper position limits (rad)",vector<double>(PERFORMERMK2_NUM_AXES,0)),
velocityLimits_prop("velocityLimits","velocity limits of the axes, (rad/s)",vector<double>(PERFORMERMK2_NUM_AXES,0)),
initialPosition_prop("initialPosition","Initial position (rad) for simulation or hardware",vector<double>(PERFORMERMK2_NUM_AXES,0)),
driveOffset_prop("driveOffset","offset (in rad/s) to the drive value.",vector<double>(PERFORMERMK2_NUM_AXES,0)),
simulation_prop("simulation","true if simulationAxes should be used",true),
simulation(true),
//servoIntegrationFactor_prop("ServoIntegrationFactor","Inverse of Integration time for servoloop",vector<double>(PERFORMERMK2_NUM_AXES,0)),
//servoGain_prop("ServoGain","Feedback Gain for servoloop",vector<double>(PERFORMERMK2_NUM_AXES,0)),
servoFFScale_prop("ServoFFScale","Feedforward scale for servoloop",vector<double>(PERFORMERMK2_NUM_AXES,0)),
PIDkp_prop("PIDkp","PID proportional gain",vector<double>(PERFORMERMK2_NUM_AXES,0)),
PIDTi_prop("PIDTi","PID integration time",vector<double>(PERFORMERMK2_NUM_AXES,0)),
PIDTd_prop("PIDTd","PID differentiation time",vector<double>(PERFORMERMK2_NUM_AXES,0)),
num_axes_attr("NUM_AXES",PERFORMERMK2_NUM_AXES),
chain_attr("Kinematics"),
driveOutOfRange_evt("driveOutOfRange"),
positionOutOfRange_evt("positionOutOfRange"),
velocityOutOfRange_evt("velocityOutOfRange"),
propertyfile(_propertyfile),
activated(false),
positionConvertFactor(PERFORMERMK2_NUM_AXES,0),
driveConvertFactor(PERFORMERMK2_NUM_AXES,0),
positionValues(PERFORMERMK2_NUM_AXES,0),
driveValues(PERFORMERMK2_NUM_AXES,0),
velocityValues(PERFORMERMK2_NUM_AXES,0),
jValues(PERFORMERMK2_NUM_AXES,0),
positionDeque_axis1(m,0),
positionDeque_axis2(m,0),
positionDeque_axis3(m,0),
positionDeque_axis4(m,0),
positionDeque_axis5(m,0),
timeDeque(m,0),
homingSwitchValues(PERFORMERMK2_NUM_AXES,0),
outputvel(PERFORMERMK2_NUM_AXES,0),
outputvel_kmin1(PERFORMERMK2_NUM_AXES,0),
outputvel_kmin2(PERFORMERMK2_NUM_AXES,0),
velocity_error_k(PERFORMERMK2_NUM_AXES,0),
velocity_error_kmin1(PERFORMERMK2_NUM_AXES,0),
velocity_error_kmin2(PERFORMERMK2_NUM_AXES,0),
position_error_k(PERFORMERMK2_NUM_AXES,0),
position_desired_k(PERFORMERMK2_NUM_AXES,0),
#if (defined OROPKG_OS_LXRT)
axes_hardware(PERFORMERMK2_NUM_AXES),
encoderInterface(PERFORMERMK2_NUM_AXES),
encoder(PERFORMERMK2_NUM_AXES),
vref(PERFORMERMK2_NUM_AXES),
enable(PERFORMERMK2_NUM_AXES),
drive(PERFORMERMK2_NUM_AXES),
homingSwitch(PERFORMERMK2_NUM_AXES),
#endif
axes(PERFORMERMK2_NUM_AXES)
//_previous_time(PERFORMERMK2_NUM_AXES)
{
Logger::In in(this->getName().data());
double ticks2rad[PERFORMERMK2_NUM_AXES] = PERFORMERMK2_TICKS2RAD;
double vel2volt[PERFORMERMK2_NUM_AXES] = PERFORMERMK2_RADproSEC2VOLT;
for(unsigned int i = 0;i<PERFORMERMK2_NUM_AXES;i++){
positionConvertFactor[i] = ticks2rad[i];
driveConvertFactor[i] = vel2volt[i];
}

#if (defined OROPKG_OS_LXRT)
	double encoderOffsets[PERFORMERMK2_NUM_AXES] = PERFORMERMK2_ENCODEROFFSETS;
	
	log(Info)<<"Creating Comedi Devices."<<endlog();
	NI6713 = new ComediDevice(0);
	NI6602 = new ComediDevice(2);
	
	SubAOut_NI6713 = new ComediSubDeviceAOut(NI6713,"DriveValues",1);
	SubDIn_NI6713 = new ComediSubDeviceDIn(NI6713,"DigitalIn",2);
	SubDOut_NI6602 = new ComediSubDeviceDOut(NI6602,"DigitalOut",1);
	
	brakeAxis2 = new DigitalOutput(SubDOut_NI6602,0,true);
	brakeAxis2->switchOn();
	brakeAxis3 = new DigitalOutput(SubDOut_NI6602,1,true);
	brakeAxis3->switchOn();
		
	armPowerOn = new DigitalInput(SubDIn_NI6713,6);
	armPowerEnable = new DigitalOutput(SubDOut_NI6602,7);
	armPowerEnable->switchOff();
		
	
	for (unsigned int i = 0; i < PERFORMERMK2_NUM_AXES; i++){
	log(Info)<<"Creating Hardware axis "<<i<<endlog();
	//Setting up encoders
	log(Info)<<"Setting up encoder ..."<<endlog();
	encoderInterface[i] = new ComediEncoder(NI6602,0,i);
	encoder[i] = new IncrementalEncoderSensor( encoderInterface[i], 1.0 / ticks2rad[i],
						encoderOffsets[i],
						-10, 10,0 );
	//Setting up drive            
	log(Info)<<"Setting up drive ..."<<endlog();
	//	    if(i==2)
	//	      vref[i]   = new AnalogOutput<unsigned int>(SubAOut_NI6713, 5 );
	//else
	vref[i]   = new AnalogOutput<unsigned int>(SubAOut_NI6713, i );
	enable[i] = new DigitalOutput( SubDOut_NI6602, 2+i );
	enable[i]->switchOff();
	drive[i]  = new AnalogDrive( vref[i], enable[i], 1.0 / vel2volt[i], 0.0);
	
	axes_hardware[i] = new Axis( drive[i] );
	axes_hardware[i]->setSensor( "Position", encoder[i] );
	if(i==1)
	axes_hardware[i]->setBrake(brakeAxis2);
	if(i==2)
	axes_hardware[i]->setBrake(brakeAxis3);
	
	//Setting up homeswitches
	homingSwitch[i] = new DigitalInput(SubDIn_NI6713,i);
	axes_hardware[i]->setSwitch("HomingSwitch",homingSwitch[i]);
	
	}
	
#endif
	//Definition of kinematics for the PerformerMK2 
	kinematics.addSegment(Segment(Joint(Joint::RotZ),Frame(Vector(0.100,0.0,0.360))));
	kinematics.addSegment(Segment(Joint(Joint::RotY),Frame(Vector(0.0,0.0,0.270))));
	kinematics.addSegment(Segment(Joint(Joint::RotY),Frame(Vector(0.0,0.0,0.230))));
	kinematics.addSegment(Segment(Joint(Joint::RotY),Frame(Vector(0.0,0.0,0.100))));
	kinematics.addSegment(Segment(Joint(Joint::RotZ)));
	
	chain_attr.set(kinematics);
	
	/*
	*  Execution Interface
	*/
	methods()->addMethod( &resetController_mtd, "reset Velocity Controller","axis_id","axis to reset");
	methods()->addMethod( &startAllAxes_mtd, "start all axes"  );
	methods()->addMethod( &stopAllAxes_mtd, "stops all axes"  );
	methods()->addMethod( &lockAllAxes_mtd, "locks all axes"  );
	methods()->addMethod( &unlockAllAxes_mtd, "unlock all axes"  );
	commands()->addCommand( &prepareForUse_cmd, "prepares the robot for use"  );
	commands()->addCommand( &prepareForShutdown_cmd,"prepares the robot for shutdown"  );
	methods()->addMethod( &addDriveOffset_mtd,"adds an offset to the drive value of axis","offset","offset values in rad/s" );
	methods()->addMethod( &_initPosition,"changes position value to the initial position","axis","axis to initialize" );
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
	ports()->addPort(&jValues_port);
	ports()->addPort(&homingSwitchValues_port);
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
	//properties()->addProperty( &servoIntegrationFactor_prop);
	//properties()->addProperty( &servoGain_prop);
	properties()->addProperty( &servoFFScale_prop);
	properties()->addProperty( &PIDkp_prop);
	properties()->addProperty( &PIDTi_prop);
	properties()->addProperty( &PIDTd_prop);
	attributes()->addConstant( &num_axes_attr);
	attributes()->addAttribute(&chain_attr);

}

PerformerMK2nAxesVelocityController::~PerformerMK2nAxesVelocityController()
{
	// make sure robot is shut down
	prepareForShutdown_cmd();

	// brake, drive, sensors and switches are deleted by each axis
	if(simulation)
	for (unsigned int i = 0; i < PERFORMERMK2_NUM_AXES; i++)
		delete axes[i];

#if (defined OROPKG_OS_LXRT)
	for (unsigned int i = 0; i < PERFORMERMK2_NUM_AXES; i++){
	delete axes_hardware[i];
	}
	delete armPowerOn;
	delete armPowerEnable;
	
	delete SubAOut_NI6713;
	delete SubDIn_NI6713;
	delete SubDOut_NI6602;
	
	delete NI6602;
	delete NI6713;
#endif
}


bool PerformerMK2nAxesVelocityController::configureHook()
{
Logger::In in(this->getName().data());
	
if (!marshalling()->readProperties(propertyfile)) {
	log(Error) << "Failed to read the property file, continueing with default values." << endlog();
	return false;
	}  
	simulation=simulation_prop.value();

	if(!(driveLimits_prop.value().size()==PERFORMERMK2_NUM_AXES&&
	lowerPositionLimits_prop.value().size()==PERFORMERMK2_NUM_AXES&&
	upperPositionLimits_prop.value().size()==PERFORMERMK2_NUM_AXES&&
	velocityLimits_prop.value().size()==PERFORMERMK2_NUM_AXES&&
	initialPosition_prop.value().size()==PERFORMERMK2_NUM_AXES&&
	driveOffset_prop.value().size()==PERFORMERMK2_NUM_AXES)){
	log(Error) << "Properties of invalid size" << endlog();
	return false;
	}
	
#if (defined OROPKG_OS_LXRT)
	if(!simulation){

	for (unsigned int i = 0; i <PERFORMERMK2_NUM_AXES; i++){
	axes_hardware[i]->limitDrive(-driveLimits_prop.value()[i], driveLimits_prop.value()[i], driveOutOfRange_evt);
	axes[i] = axes_hardware[i];
	((Axis*)(axes[i]))->getDrive()->addOffset(driveOffset_prop.value()[i]);
	log(Info) << "Hardware version of PerformerMK2nAxesVelocityController has started" << endlog();
	}

	}
	else{
#endif
	for (unsigned int i = 0; i <PERFORMERMK2_NUM_AXES; i++)
		axes[i] = new SimulationAxis(initialPosition_prop.value()[i],
					lowerPositionLimits_prop.value()[i],
					upperPositionLimits_prop.value()[i]);
	log(Info) << "Simulation version of PerformerMK2nAxesVelocityController has started" << endlog();
#if (defined OROPKG_OS_LXRT)
	}
#endif
	/**
	* Initializing servoloop
	*/
	//servoGain = servoGain_prop.value();
	//servoIntegrationFactor = servoIntegrationFactor_prop.value();
	servoFFScale = servoFFScale_prop.value();

	/**
	* Initializing PID velocity controllers
	*/

	PIDkp = PIDkp_prop.value();
	PIDTi = PIDTi_prop.value();
	PIDTd = PIDTd_prop.value();
	
	return true;
}

bool PerformerMK2nAxesVelocityController::startHook()
{
  previous_time = TimeService::Instance()->getTicks();

  for (int axis=0;axis<PERFORMERMK2_NUM_AXES;axis++) {
    positionValues[axis] = axes[axis]->getSensor("Position")->readSensor();
    position_desired_k[axis] = position_desired_k[axis];
    if(!simulation)
      homingSwitchValues[axis] = axes[axis]->getSwitch("HomingSwitch")->isOn();
    else
      homingSwitchValues[axis] = true;
    
  }

	for (int i=0;i<m;i++) {
	positionDeque_axis1[i] = positionValues[0];
	positionDeque_axis2[i] = positionValues[1];
	positionDeque_axis3[i] = positionValues[2];
	positionDeque_axis4[i] = positionValues[3];
	positionDeque_axis5[i] = positionValues[4];
	timeDeque[i] = 0.0;
	}


	positionValues_port.Set(positionValues);
	homingSwitchValues_port.Set(homingSwitchValues);
	return true;
}

void PerformerMK2nAxesVelocityController::updateHook()
{
delta_time = TimeService::Instance()->secondsSince(previous_time);
total_time = TimeService::Instance()->secondsSince(initial_time);
previous_time = TimeService::Instance()->getTicks();
deltaTime_port.Set(delta_time);

for (int axis=0;axis<PERFORMERMK2_NUM_AXES;axis++) {
	// Euler approximation velocity estimator	
	// velocityValues[axis] = (axes[axis]->getSensor("Position")->readSensor() - positionValues[axis])/delta_time;
	positionValues[axis] = axes[axis]->getSensor("Position")->readSensor();
	if(!simulation)
	  homingSwitchValues[axis] = axes[axis]->getSwitch("HomingSwitch")->isOn();
	else
	  homingSwitchValues[axis] = true;
        }

   	positionDeque_axis1.push_front(positionValues[0]);
	positionDeque_axis1.pop_back();
	positionDeque_axis2.push_front(positionValues[1]);
	positionDeque_axis2.pop_back();
	positionDeque_axis3.push_front(positionValues[2]);
	positionDeque_axis3.pop_back();
	positionDeque_axis4.push_front(positionValues[3]);
	positionDeque_axis4.pop_back();
	positionDeque_axis5.push_front(positionValues[4]);
	positionDeque_axis5.pop_back();
	timeDeque.push_front(total_time);
	timeDeque.pop_back();
	
	for (int j=1;j<m;j++) {
		velocityValues[0] = (positionDeque_axis1[0]-positionDeque_axis1[m-1])/(timeDeque[0]-timeDeque[m-1]);
		if(abs(positionDeque_axis1[0]-positionDeque_axis1[j])>sj*R1){
			velocityValues[0] = (positionDeque_axis1[0]-positionDeque_axis1[j])/(timeDeque[0]-timeDeque[j]);
			jValues[0] = j;
			break;
		}
	}

	for (int j=1;j<m;j++) {
		velocityValues[1] = (positionDeque_axis2[0]-positionDeque_axis2[m-1])/(timeDeque[0]-timeDeque[m-1]);
		if(abs(positionDeque_axis2[0]-positionDeque_axis2[j])>sj*R2){
			velocityValues[1] = (positionDeque_axis2[0]-positionDeque_axis2[j])/(timeDeque[0]-timeDeque[j]);
			jValues[1] = j;
			break;
		}
	}

	for (int j=1;j<m;j++) {
		velocityValues[2] = (positionDeque_axis3[0]-positionDeque_axis3[m-1])/(timeDeque[0]-timeDeque[m-1]);
		if(abs(positionDeque_axis3[0]-positionDeque_axis3[j])>sj*R3){
			velocityValues[2] = (positionDeque_axis3[0]-positionDeque_axis3[j])/(timeDeque[0]-timeDeque[j]);
			jValues[2] = j;
			break;
		}
	}

	for (int j=1;j<m;j++) {
		velocityValues[3] = (positionDeque_axis4[0]-positionDeque_axis4[m-1])/(timeDeque[0]-timeDeque[m-1]);
		if(abs(positionDeque_axis4[0]-positionDeque_axis4[j])>sj*R4) {
			velocityValues[3] = (positionDeque_axis4[0]-positionDeque_axis4[j])/(timeDeque[0]-timeDeque[j]);
			jValues[3] = j;
			break;
		}
	}

	for (int j=1;j<m;j++) {
		velocityValues[4] = (positionDeque_axis5[0]-positionDeque_axis5[m-1])/(timeDeque[0]-timeDeque[m-1]);
		if(abs(positionDeque_axis5[0]-positionDeque_axis5[j])>sj*R5){
			velocityValues[4] = (positionDeque_axis5[0]-positionDeque_axis5[j])/(timeDeque[0]-timeDeque[j]);
			jValues[4] = j;
			break;
		}
	}

 
	positionValues_port.Set(positionValues);
	velocityValues_port.Set(velocityValues);
	jValues_port.Set(jValues);
	homingSwitchValues_port.Set(homingSwitchValues);

#if defined OROPKG_OS_LXRT
	//double dt;
	// Determine sampling time :
	//if (servoInitialized) {
	//  dt              = TimeService::Instance()->secondsSince(previousTime);
	//  previousTime    = TimeService::Instance()->getTicks();
	//} else {
	//  dt = 0.0; 
	//  for (unsigned int i=0;i<PERFORMERMK2_NUM_AXES;i++) {      
	//    servoIntError[i] = 0.0;
	//  }
	//  previousTime = TimeService::Instance()->getTicks();
	//  servoInitialized = true;
	//}
#endif
	driveValues_port.Get(driveValues);
	
	for (unsigned int i=0;i<PERFORMERMK2_NUM_AXES;i++) {      
	// emit event when velocity is out of range
	if( (velocityValues[i] < -velocityLimits_prop.value()[i]) ||
	(velocityValues[i] > velocityLimits_prop.value()[i]) )
	{
	stringstream msg;
	msg<<"Velocity of PerformerMK2 Axis "<< i <<" is out of range: "<<velocityValues[i];
	log(Warning)<<msg.str()<<endlog();
	velocityOutOfRange_evt(msg.str()+"\n");
	}
	// emit event when position is out of range
	if( (positionValues[i] < lowerPositionLimits_prop.value()[i]) ||
	(positionValues[i] > upperPositionLimits_prop.value()[i]) ){
	stringstream msg;
	msg<<"Position of PerformerMK2 Axis "<<i<<" is out of range: "<<positionValues[i];
	log(Warning)<<msg.str()<<endlog();
	positionOutOfRange_evt(msg.str()+"\n");
	}
	
	// send the drive value to hw and performs checks
	if (axes[i]->isDriven()){
#if defined OROPKG_OS_LXRT      
	  // perform control action ( dt is zero the first time !) :
	  //   double error        = driveValues[i] - velocityValues[i];
	  // servoIntError[i]    += dt*error;
	  //outputvel[i] = servoGain[i]*(error + servoIntegrationFactor[i]*servoIntError[i]) + servoFFScale[i]*driveValues[i];
	  if ((i==0 || i==3 || i==4)&&!simulation ){ 
	    velocity_error_kmin2[i] = velocity_error_kmin1[i];
	    velocity_error_kmin1[i] = velocity_error_k[i];
	    velocity_error_k[i] = driveValues[i]-velocityValues[i];
	    position_desired_k[i] = position_desired_k[i] + driveValues[i]*delta_time;
	    position_error_k[i] = position_desired_k[i]-positionValues[i];
	    outputvel_kmin2[i] =  outputvel_kmin1[i];
	    outputvel_kmin1[i] =  outputvel[i];
	    
	    // outputvel by discretizing PID controller (design by emulation)
	    //outputvel[i] = PIDkp[i]*(velocity_error_k[i]-velocity_error_kmin2[i])+(1/2)*delta_time*velocity_error_k[i]*PIDkp[i]/PIDTi[i]+delta_time*velocity_error_kmin1[i]*PIDkp[i]/PIDTi[i]+(1/2)*delta_time*velocity_error_kmin2[i]*PIDkp[i]/(PIDTi[i])+2*velocity_error_k[i]*PIDkp[i]*PIDTd[i]/delta_time-4*velocity_error_kmin1[i]*PIDkp[i]*PIDTd[i]/delta_time+2*velocity_error_kmin2[i]*PIDkp[i]*PIDTd[i]/delta_time+outputvel_kmin2[i];
	    
	    // outputvel for PI controller:
	    outputvel[i] = PIDkp[i]*(velocity_error_k[i]+1.0/PIDTi[i]*position_error_k[i]);
	    
	  }
	  else{
	    outputvel[i] = 0.0;
	  }
	  
	  driveValues[i] = outputvel[i]+servoFFScale[i]*driveValues[i];
	} 
	else {
	  outputvel[i] = 0.0;
	  driveValues[i] = outputvel[i];
	}
	axes[i]->drive(driveValues[i]);
#else
	axes[i]->drive(driveValues[i]);
	}
	
#endif
}
servoValues_port.Set(driveValues);
}



void PerformerMK2nAxesVelocityController::stopHook()
{
	//Make sure machine is shut down
	prepareForShutdown();
}

void PerformerMK2nAxesVelocityController::cleanupHook()
{
	//Write properties back to file
	marshalling()->writeProperties(propertyfile);
}

bool PerformerMK2nAxesVelocityController::resetController(const int& axis_id)
{
	bool succes = false;
    	if(axis_id < 5 && axis_id >= 0){
    		succes = true;
		velocity_error_kmin2[axis_id] = 0.0;
		velocity_error_kmin1[axis_id] = 0.0;
		velocity_error_k[axis_id] = 0.0;
    		outputvel_kmin1[axis_id] = 0.0;
    		outputvel_kmin2[axis_id] = 0.0;
		positionValues[axis_id] = axes[axis_id]->getSensor("Position")->readSensor();
		position_desired_k[axis_id] = positionValues[axis_id];

		if (axis_id == 0){
			for (int i=0;i<m;i++) {
				positionDeque_axis1[i] = positionValues[0];
				}
		}
		if (axis_id == 1){
			for (int i=0;i<m;i++) {
				positionDeque_axis2[i] = positionValues[1];
				}
		}
    		if (axis_id == 2){
			for (int i=0;i<m;i++) {
				positionDeque_axis3[i] = positionValues[2];
				}
		}
		if (axis_id == 3){
			for (int i=0;i<m;i++) {
				positionDeque_axis4[i] = positionValues[3];
				}
		}
    		if (axis_id == 4){
			for (int i=0;i<m;i++) {
				positionDeque_axis5[i] = positionValues[4];
				}
		}
	}

    	return succes;
}
	
bool PerformerMK2nAxesVelocityController::prepareForUse()
{
#if (defined OROPKG_OS_LXRT)
if(!simulation){
	armPowerEnable->switchOn();
	log(Warning) <<"Release Emergency stop and push button to start ...."<<endlog();
}
#endif
activated = true;
return true;
}

bool PerformerMK2nAxesVelocityController::prepareForUseCompleted()const
{
#if (defined OROPKG_OS_LXRT)
	if(!simulation)
	return armPowerOn->isOn();
	else
#endif
	{
	
	return true;
	}
	
}

bool PerformerMK2nAxesVelocityController::prepareForShutdown()
{
  //make sure all axes are stopped and locked
  stopAllAxes();
  lockAllAxes();
#if (defined OROPKG_OS_LXRT)
  armPowerEnable->switchOff();
#endif
  activated = false;
  return true;
}

bool PerformerMK2nAxesVelocityController::prepareForShutdownCompleted()const
{
	return true;
}

bool PerformerMK2nAxesVelocityController::stopAllAxes()
{
	bool succes = true;
	for(unsigned int i = 0;i<PERFORMERMK2_NUM_AXES;i++)
	succes &= axes[i]->stop();
	
	return succes;
}



bool PerformerMK2nAxesVelocityController::startAllAxes()
{
	bool succes = true;
	for(unsigned int i = 0;i<PERFORMERMK2_NUM_AXES;i++)
	succes &= axes[i]->drive(0.0);
	if(!succes)
	stopAllAxes();
	
	return succes;
}

bool PerformerMK2nAxesVelocityController::unlockAllAxes()
{
	if(!activated)
	return false;
	
	bool succes = true;
	for(unsigned int i = 0;i<PERFORMERMK2_NUM_AXES;i++)
	succes &= axes[i]->unlock();
	if(!succes)
	lockAllAxes();
	
	return succes;
}

bool PerformerMK2nAxesVelocityController::lockAllAxes()
{
	bool succes = true;
	for(unsigned int i = 0;i<PERFORMERMK2_NUM_AXES;i++){
	succes &= axes[i]->lock();
	}
	return succes;
}

bool PerformerMK2nAxesVelocityController::addDriveOffset(const vector<double>& offset)
{
	if(offset.size()!=PERFORMERMK2_NUM_AXES)
	return false;
	
	for(unsigned int i=0;i<PERFORMERMK2_NUM_AXES;i++){
	driveOffset_prop.value()[i] += offset[i];
#if (defined OROPKG_OS_LXRT)
	if (!simulation)
		((Axis*)(axes[i]))->getDrive()->addOffset(offset[i]);
#endif
	}
	return true;
}
    bool PerformerMK2nAxesVelocityController::initPosition(int axis)
    {
#if (defined OROPKG_OS_LXRT)
        if(!simulation)
            ((IncrementalEncoderSensor*)axes[axis]->getSensor("Position"))->writeSensor(initialPosition_prop.value()[axis]);
#endif
        return true;
    }
    
    bool PerformerMK2nAxesVelocityController::initPositionCompleted(int axis)const
    {
        return true;
    }


}//namespace orocos
ORO_CREATE_COMPONENT(OCL::PerformerMK2nAxesVelocityController)
