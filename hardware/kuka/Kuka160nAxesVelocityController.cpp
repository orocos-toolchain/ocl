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
#include <ocl/ComponentLoader.hpp>

#include <rtt/Logger.hpp>


namespace OCL{
using namespace RTT;
using namespace std;
using namespace KDL;

#define KUKA160_NUM_AXES 6
#define KUKA160_CONV1  -120*114*106*100/( 30*40*48*14)
#define KUKA160_CONV2  -168*139*111/(28*37*15)
#define KUKA160_CONV3  -168*125*106/(28*41*15)
#define KUKA160_CONV4  -150.857
#define KUKA160_CONV5  -155.17
#define KUKA160_CONV6  -100

  // Resolution of the encoders
#define KUKA160_ENC_RES  4096

  // Conversion from encoder ticks to radiants
#define KUKA160_TICKS2RAD { 2*M_PI / (KUKA160_CONV1 * KUKA160_ENC_RES), 2*M_PI / (KUKA160_CONV2 * KUKA160_ENC_RES), 2*M_PI / (KUKA160_CONV3 * KUKA160_ENC_RES), 2*M_PI / (KUKA160_CONV4 * KUKA160_ENC_RES), 2*M_PI / (KUKA160_CONV5 * KUKA160_ENC_RES), 2*M_PI / (KUKA160_CONV6 * KUKA160_ENC_RES)}

  // Conversion from angular speed to voltage
#define KUKA160_RADproSEC2VOLT { -3.97143, -4.40112, -3.65062, -3.38542, -4.30991, -2.75810 }

    typedef Kuka160nAxesVelocityController MyType;

    Kuka160nAxesVelocityController::Kuka160nAxesVelocityController(string name)
        : TaskContext(name,PreOperational),
          driveValues_port("nAxesOutputVelocity"),
          referenceValues_port("ReferenceValues"),
          positionValues_port("nAxesSensorPosition"),
          driveLimits_prop("driveLimits","velocity limits of the axes, (rad/s)",std::vector<double>(KUKA160_NUM_AXES,0)),
          lowerPositionLimits_prop("LowerPositionLimits","Lower position limits (rad)",std::vector<double>(KUKA160_NUM_AXES,0)),
          upperPositionLimits_prop("UpperPositionLimits","Upper position limits (rad)",std::vector<double>(KUKA160_NUM_AXES,0)),
          initialPosition_prop("initialPosition","Initial position (rad) for simulation or hardware",std::vector<double>(KUKA160_NUM_AXES,0)),
          driveOffset_prop("driveOffset","offset (in rad/s) to the drive value.",std::vector<double>(KUKA160_NUM_AXES,0)),
          simulation_prop("simulation","true if simulationAxes should be used",true),
          simulation(true),
          EmergencyEvents_prop("EmergencyEvents","List of events that will result in an emergencystop of the robot"),
          num_axes_attr("KUKA160_NUM_AXES",KUKA160_NUM_AXES),
          chain_attr("Kinematics"),
          driveOutOfRange_evt("driveOutOfRange"),
          positionOutOfRange_evt("positionOutOfRange"),
          activated(false),
          positionConvertFactor(KUKA160_NUM_AXES),
          driveConvertFactor(KUKA160_NUM_AXES),
          driveValues(KUKA160_NUM_AXES),
          positionValues(KUKA160_NUM_AXES),
          references(KUKA160_NUM_AXES),
#if (defined OROPKG_OS_LXRT)
          encoderInterface(KUKA160_NUM_AXES),
          vref(KUKA160_NUM_AXES),
          encoder(KUKA160_NUM_AXES),
          enable(KUKA160_NUM_AXES),
          drive(KUKA160_NUM_AXES),
          brake(KUKA160_NUM_AXES),
          reference(KUKA160_NUM_AXES),
          axes_hardware(KUKA160_NUM_AXES),
#endif
          axes(KUKA160_NUM_AXES),
          axes_simulation(KUKA160_NUM_AXES)
    {
        Logger::In in(this->getName().data());

        double ticks2rad[KUKA160_NUM_AXES] = KUKA160_TICKS2RAD;
        double vel2volt[KUKA160_NUM_AXES] = KUKA160_RADproSEC2VOLT;
        for(unsigned int i = 0;i<KUKA160_NUM_AXES;i++){
            positionConvertFactor[i] = ticks2rad[i];
            driveConvertFactor[i] = vel2volt[i];
        }

#if (defined OROPKG_OS_LXRT)
        comediDevAOut       = new ComediDevice( 0 );
        comediDevDInOut     = new ComediDevice( 3 );
        comediDevEncoder    = new ComediDevice( 2 );
        log(Info)<< "ComediDevices Created"<<endlog();

        int subd;
        subd = 1; // subdevice 1 is analog out
        comediSubdevAOut    = new ComediSubDeviceAOut( comediDevAOut, "Kuka160", subd );
        subd = 0; // subdevice 0 is digital in
        comediSubdevDIn     = new ComediSubDeviceDIn( comediDevDInOut, "Kuka160", subd );
        subd = 1; // subdevice 1 is digital out
        comediSubdevDOut    = new ComediSubDeviceDOut( comediDevDInOut, "Kuka160", subd );
        log(Info)<<"ComediSubDevices Created"<<endlog();

        // first switch all channels off
        for(int i = 0; i < 24 ; i++)  comediSubdevDOut->switchOff( i );

        for (unsigned int i = 0; i < KUKA160_NUM_AXES; i++){
            log(Info)<<"Kuka160 Creating Hardware Axis "<<i<<endlog();
            //Setting up encoders
            log(Info)<<"Setting up encoder ..."<<endlog();
            subd = 0; // subdevice 0 is counter
            encoderInterface[i] = new ComediEncoder(comediDevEncoder , subd , i);
            log(Info)<<"Creation ComediEncoder succeeded."<<endlog();

            log(Info)<<"encoder settings: "<<positionConvertFactor[i]<<endlog();
            log(Info)<<", "<<initialPosition_prop.value()[i]<<endlog();
            log(Info)<<", "<<lowerPositionLimits_prop.value()[i]<<endlog();
            log(Info)<<", "<<upperPositionLimits_prop.value()[i]<<endlog();

            encoder[i] = new IncrementalEncoderSensor( encoderInterface[i], 1.0 / positionConvertFactor[i],
                                                       initialPosition_prop.value()[i]*positionConvertFactor[i],
                                                       lowerPositionLimits_prop.value()[i], upperPositionLimits_prop.value()[i],KUKA160_ENC_RES);
            log(Info)<<"Setting up brake ..."<<endlog();
            brake[i] = new DigitalOutput( comediSubdevDOut, 23 - i,true);
            brake[i]->switchOn();

            vref[i]   = new AnalogOutput( comediSubdevAOut, i );
            enable[i] = new DigitalOutput( comediSubdevDOut, 13 - i );
            reference[i] = new DigitalInput( comediSubdevDIn, 23 - i);
            drive[i]  = new AnalogDrive( vref[i], enable[i], 1.0 / vel2volt[i], driveOffset_prop.value()[i]);

            axes_hardware[i] = new Axis( drive[i] );
            axes_hardware[i]->setBrake( brake[i] );
            axes_hardware[i]->setSensor( "Position", encoder[i] );
            axes_hardware[i]->setSwitch( "Reference", reference[i]);

        }

#endif
        //Definition of kinematics for the Kuka160
        kinematics.addSegment(Segment(Joint(Joint::RotZ),Frame(Vector(0.0,0.0,0.9))));
        kinematics.addSegment(Segment(Joint(Joint::RotX),Frame(Vector(0.0,0.0,0.97))));
        kinematics.addSegment(Segment(Joint(Joint::RotX),Frame(Vector(0.0,0.0,1.080))));
        kinematics.addSegment(Segment(Joint(Joint::RotZ)));
        kinematics.addSegment(Segment(Joint(Joint::RotX)));
        kinematics.addSegment(Segment(Joint(Joint::RotZ),Frame(Vector(0.0,0.0,0.18))));

        chain_attr.set(kinematics);


        /*
         *  Command Interface
         */

        this->methods()->addMethod( RTT::method("startAllAxes",&MyType::startAllAxes,this),
                                    "start all axes"  );
        this->methods()->addMethod( RTT::method("stopAllAxes",&MyType::stopAllAxes,this),
                                    "stops all axes"  );
        this->methods()->addMethod( RTT::method("lockAllAxes",&MyType::lockAllAxes,this),
                                                "locks all axes"  );
        this->methods()->addMethod( RTT::method("unlockAllAxes",&MyType::unlockAllAxes,this),
                                    "unlock all axes"  );
        this->methods()->addMethod( RTT::method("addDriveOffset",&MyType::addDriveOffset,this),
                                    "adds offset to the drive values",
                                    "offset","offset value in rad/s" );
        this->methods()->addMethod( RTT::method("initPosition",&MyType::initPosition,this),
                                    "changes position value of axis to the initial position",
                                    "switchposition","recorded switchpositions");

        this->commands()->addCommand(RTT::command("prepareForUse",&MyType::prepareForUse,&MyType::prepareForUseCompleted,this),
                                     "prepares the robot for use"  );
        this->commands()->addCommand(RTT::command("prepareForShutdown",&MyType::prepareForShutdown,&MyType::prepareForShutdownCompleted,this),
                                     "prepares the robot for shutdown"  );

        /**
         * Creating and adding the data-ports
         */
        ports()->addPort(&driveValues_port);
        ports()->addPort(&positionValues_port);
        ports()->addPort(&referenceValues_port);

        /**
         * Adding the events :
         */
        events()->addEvent( &driveOutOfRange_evt, "Velocity of Axis is out of range","message","Information about event" );
        events()->addEvent( &positionOutOfRange_evt, "Position of an Axis is out of range","message","Information about event");


        properties()->addProperty( &driveLimits_prop );
        properties()->addProperty( &lowerPositionLimits_prop );
        properties()->addProperty( &upperPositionLimits_prop  );
        properties()->addProperty( &initialPosition_prop  );
        properties()->addProperty( &driveOffset_prop  );
        properties()->addProperty( &simulation_prop  );
        properties()->addProperty( &EmergencyEvents_prop);
        attributes()->addConstant( &num_axes_attr);
        attributes()->addAttribute(&chain_attr);




    }

    Kuka160nAxesVelocityController::~Kuka160nAxesVelocityController()
    {
        this->cleanup();

        // brake, drive, sensors and switches are deleted by each axis
        if(simulation)
            for (unsigned int i = 0; i < KUKA160_NUM_AXES; i++)
                delete axes[i];

#if (defined OROPKG_OS_LXRT)
        for (unsigned int i = 0; i < KUKA160_NUM_AXES; i++)
            delete axes_hardware[i];
        delete comediDevAOut;
        delete comediDevDInOut;
        delete comediDevEncoder;
        delete comediSubdevAOut;
        delete comediSubdevDIn;
        delete comediSubdevDOut;
#endif
    }
    bool Kuka160nAxesVelocityController::configureHook()
    {
        Logger::In in(this->getName().data());

        simulation=simulation_prop.value();

        if(!(driveLimits_prop.value().size()==KUKA160_NUM_AXES&&
             lowerPositionLimits_prop.value().size()==KUKA160_NUM_AXES&&
             upperPositionLimits_prop.value().size()==KUKA160_NUM_AXES&&
             initialPosition_prop.value().size()==KUKA160_NUM_AXES&&
             driveOffset_prop.value().size()==KUKA160_NUM_AXES))
            return false;

#if (defined OROPKG_OS_LXRT)
        if(!simulation){
            for (unsigned int i = 0; i <KUKA160_NUM_AXES; i++){
	      axes_hardware[i]->limitDrive(-driveLimits_prop.value()[i], driveLimits_prop.value()[i], driveOutOfRange_evt);
	      axes[i] = axes_hardware[i];
	      ((Axis*)(axes[i]))->getDrive()->addOffset(driveOffset_prop.value()[i]);
            }
	    log(Info) << "Hardware version of Kuka160nAxesVelocityController has started" << endlog();
        }
        else{
#endif
            for (unsigned int i = 0; i <KUKA160_NUM_AXES; i++)
                axes[i] = new SimulationAxis(initialPosition_prop.value()[i],
                                             lowerPositionLimits_prop.value()[i],
                                             upperPositionLimits_prop.value()[i]);
            log(Info) << "Simulation version of Kuka160nAxesVelocityController has started" << endlog();
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
                Handle handle = peer->events()->setupConnection(eventname).callback(this,&Kuka160nAxesVelocityController::EmergencyStop).handle();
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

    bool Kuka160nAxesVelocityController::startHook()
    {
        return true;
    }

    void Kuka160nAxesVelocityController::updateHook()
    {

        driveValues_port.Get(driveValues);

        for (int axis=0;axis<KUKA160_NUM_AXES;axis++) {
            // Set the position and perform checks in joint space.
            positionValues[axis]=axes[axis]->getSensor("Position")->readSensor();

            // emit event when position is out of range
            if( (positionValues[axis] < lowerPositionLimits_prop.value()[axis]) ||
                (positionValues[axis] > upperPositionLimits_prop.value()[axis]) ){
                char msg[80];
                sprintf(msg,"Position of Kuka160 Axis %d is out of range: value: %f",axis,positionValues[axis]);
                positionOutOfRange_evt(msg);
            }

            // send the drive value to hw and performs checks
            if (axes[axis]->isDriven())
                axes[axis]->drive(driveValues[axis]);

            // ask the reference value from the hw
#if (defined OROPKG_OS_LXRT)
            if(!simulation)
                references[axis]=axes[axis]->getSwitch("Reference")->isOn();
            else
#endif
                references[axis]=false;
        }
        positionValues_port.Set(positionValues);
        referenceValues_port.Set(references);

    }

    void Kuka160nAxesVelocityController::stopHook()
    {
        //Make sure machine is shut down
        prepareForShutdown();
    }

    void Kuka160nAxesVelocityController::cleanupHook()
    {
    }

    bool Kuka160nAxesVelocityController::prepareForUse()
    {
#if (defined OROPKG_OS_LXRT)
        if(!simulation){
            comediSubdevDOut->switchOn( 17 );
            log(Warning) <<"Release Emergency stop of Kuka 160 and push button to start ...."<<endlog();
        }
#endif
        activated = true;
        return true;
    }

    bool Kuka160nAxesVelocityController::prepareForUseCompleted()const
    {
#if (defined OROPKG_OS_LXRT)
        if(!simulation)
            return (comediSubdevDIn->isOn(3) && comediSubdevDIn->isOn(5));
#endif
        return true;
    }

    bool Kuka160nAxesVelocityController::prepareForShutdown()
    {
        //make sure all axes are stopped and locked
        stopAllAxes();
        lockAllAxes();
#if (defined OROPKG_OS_LXRT)
        if(!simulation)
            comediSubdevDOut->switchOff( 17 );
#endif
        activated = false;
        return true;
    }

    bool Kuka160nAxesVelocityController::prepareForShutdownCompleted()const
    {
        return true;
    }

    bool Kuka160nAxesVelocityController::stopAllAxes()
    {
        bool succes = true;
        for(unsigned int i = 0;i<KUKA160_NUM_AXES;i++)
            succes &= axes[i]->stop();

        return succes;
    }

    bool Kuka160nAxesVelocityController::startAllAxes()
    {
        bool succes = true;
        for(unsigned int i = 0;i<KUKA160_NUM_AXES;i++)
            succes &= axes[i]->drive(0.0);

        return succes;
    }

    bool Kuka160nAxesVelocityController::unlockAllAxes()
    {
        if(!activated)
            return false;

        bool succes = true;
        for(unsigned int i = 0;i<KUKA160_NUM_AXES;i++)
            succes &= axes[i]->unlock();

        return succes;
    }

    bool Kuka160nAxesVelocityController::lockAllAxes()
    {
        bool succes = true;
        for(unsigned int i = 0;i<KUKA160_NUM_AXES;i++){
            succes &= axes[i]->lock();
        }
        return succes;
    }


    bool Kuka160nAxesVelocityController::addDriveOffset(const std::vector<double>& offset)
    {
        if(offset.size()!=KUKA160_NUM_AXES)
            return false;
        for(unsigned int i=0;i<KUKA160_NUM_AXES;i++){
            driveOffset_prop.value()[i] += offset[i];

#if (defined OROPKG_OS_LXRT)
            if (!simulation)
                ((Axis*)(axes[i]))->getDrive()->addOffset(offset[i]);
#endif
        }

        return true;
    }

bool Kuka160nAxesVelocityController::initPosition(const std::vector<double>& switchposition)
    {
#if (defined OROPKG_OS_LXRT)
        if(!simulation)
            for(unsigned int i=0;i<KUKA160_NUM_AXES;i++){
                double act_pos = ((IncrementalEncoderSensor*)axes[i]->getSensor("Position"))->readSensor();
                double new_pos = act_pos-switchposition[i]+initialPosition_prop.value()[i];
                ((IncrementalEncoderSensor*)axes[i]->getSensor("Position"))->writeSensor(new_pos);
            }
#endif
        return true;
    }

}//namespace orocos

ORO_LIST_COMPONENT_TYPE( OCL::Kuka160nAxesVelocityController );

