// Copyright (C) 2006 Wim Meeussen <wim dot meeussen at mech dot kuleuven dot be>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  


#include <rtt/RTT.hpp>
#include <rtt/NonPreemptibleActivity.hpp>
#include <rtt/NonRealTimeActivity.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/os/main.h>
#include <kdl/toolkit.hpp>
#include <kdl/kinfam/kuka361.hpp>
#include <kdl/kinfam/kuka160.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <reporting/FileReporting.hpp>
#include <hardware/kuka/Kuka361nAxesVelocityController.hpp>
#include <hardware/kuka/Kuka160nAxesVelocityController.hpp>
#include <hardware/kuka/EmergencyStop.hpp>
#include <hardware/wrench/WrenchSensor.hpp>
#include <motion_control/naxes/ReferenceSensor.hpp>
#include <motion_control/naxes/nAxesComponents.hpp>
#include <motion_control/cartesian/CartesianComponents.hpp>
#include "execution/execution.hpp"
#include "estimation/estimation.hpp"


using namespace RTT;
using namespace BFL;
using namespace KDL;
using namespace Orocos;
using namespace std;



int ORO_main(int arc, char* argv[])
{
    // Set log level more verbose than default,
    if ( Logger::log().getLogLevel() < Logger::Info ) {
        Logger::log().setLogLevel( Logger::Info );
        log(Info) << argv[0] << " manually raises LogLevel to 'Info'. " << endlog();
    }

    // import kdl toolkit
    Toolkit::Import( KDLToolkit );

    // robot
	Kuka160nAxesVelocityController _robot("Robot");
    NonPreemptibleActivity _robotTask(0.002, _robot.engine());
    Kuka160 _robot_kinematics;
    ReferenceSensor refsensorkuka160("Kuka160RefSensor",6);
    connectPorts(&refsensorkuka160, &_robot);

    // wrench sensor
    WrenchSensor _wrenchSensor(0.01,"Wrenchsensor", 0);
    NonPreemptibleActivity _wrenchSensorTask(0.01, _wrenchSensor.engine() );

    // naxes sensor
    nAxesSensor _nAxesSensor("nAxesSensor",6);
    NonPreemptibleActivity _nAxesSensorTask(0.01, _nAxesSensor.engine() ); 

    // naxes generator pos
    nAxesGeneratorPos _nAxesGeneratorPos("nAxesGeneratorPos",6);
    NonPreemptibleActivity _nAxesGeneratorPosTask(0.01, _nAxesGeneratorPos.engine() ); 
    connectPorts(&_nAxesGeneratorPos, &_nAxesSensor);

    // naxes generator vel
    nAxesGeneratorVel _nAxesGeneratorVel("nAxesGeneratorVel",6);
    NonPreemptibleActivity _nAxesGeneratorVelTask(0.01, _nAxesGeneratorVel.engine() ); 
    connectPorts(&_nAxesGeneratorVel, &_nAxesSensor);

    // naxes controller pos
    nAxesControllerPos _nAxesControllerPos("nAxesControllerPos",6);
    NonPreemptibleActivity _nAxesControllerPosTask(0.01, _nAxesControllerPos.engine() ); 
    connectPorts(&_nAxesControllerPos, &_nAxesGeneratorPos);
    connectPorts(&_nAxesControllerPos, &_nAxesSensor);

    // naxes controller pos vel
    nAxesControllerPosVel _nAxesControllerPosVel("nAxesControllerPosVel",6);
    NonPreemptibleActivity _nAxesControllerPosVelTask(0.01, _nAxesControllerPosVel.engine() ); 
    connectPorts(&_nAxesControllerPosVel, &_nAxesGeneratorPos);
    connectPorts(&_nAxesControllerPosVel, &_nAxesSensor);

    // naxes controller vel
    nAxesControllerVel _nAxesControllerVel("nAxesControllerVel",6);
    NonPreemptibleActivity _nAxesControllerVelTask(0.01, _nAxesControllerVel.engine() ); 
    connectPorts(&_nAxesControllerVel, &_nAxesGeneratorPos);
    connectPorts(&_nAxesControllerVel, &_nAxesGeneratorVel);
    connectPorts(&_nAxesControllerVel, &_nAxesSensor);

    // naxes effector
    nAxesEffectorVel _nAxesEffectorVel("nAxesEffectorVel",6);
    NonPreemptibleActivity _nAxesEffectorVelTask(0.01, _nAxesEffectorVel.engine() ); 
    connectPorts(&_robot, &_nAxesEffectorVel);
    connectPorts(&_nAxesEffectorVel, &_nAxesControllerPos);
    connectPorts(&_nAxesEffectorVel, &_nAxesControllerPosVel);
    connectPeers(&_nAxesEffectorVel, &_nAxesControllerVel);
    connectPeers(&_nAxesEffectorVel, &_nAxesSensor);
    connectPorts(&_nAxesSensor, &_robot);


    // cartesian sensor
    CartesianSensor _CartesianSensor("CartesianSensor", &_robot_kinematics);
    NonPreemptibleActivity _CartesianSensorTask(0.01, _CartesianSensor.engine() ); 
    connectPorts(&_CartesianSensor, &_robot);

    // cartesian generator
    CartesianGeneratorPos _CartesianGeneratorPos("CartesianGeneratorPos");
    NonPreemptibleActivity _CartesianGeneratorPosTask(0.01, _CartesianGeneratorPos.engine() ); 
    connectPorts(&_CartesianGeneratorPos, &_CartesianSensor);

    // cartesian controller pos vel
    CartesianControllerPosVel _CartesianControllerPosVel("CartesianControllerPosVel");
    NonPreemptibleActivity _CartesianControllerTask(0.01, _CartesianControllerPosVel.engine() ); 
    connectPorts(&_CartesianControllerPosVel, &_CartesianGeneratorPos);
    connectPorts(&_CartesianControllerPosVel, &_CartesianSensor);

    // cartesian effector
    CartesianEffectorVel _CartesianEffectorVel("CartesianEffectorVel", &_robot_kinematics);
    NonPreemptibleActivity _CartesianEffectorVelTask(0.01, _CartesianEffectorVel.engine() ); 
    connectPorts(&_robot, &_CartesianEffectorVel);
    connectPorts(&_CartesianEffectorVel, &_CartesianControllerPosVel);
    connectPorts(&_CartesianEffectorVel, &_CartesianSensor);

    // execution
    Execution _execution("Execution");
    NonPreemptibleActivity _executionTask(0.01, _execution.engine() );
    connectPorts(&_execution, &_CartesianSensor);
    connectPorts(&_execution, &_CartesianEffectorVel);
    connectPorts(&_execution, &_wrenchSensor);

    // estimation
    Estimation _estimation("Estimation");
    NonRealTimeActivity _estimationTask(1.0, _estimation.engine() );
    connectPorts(&_estimation, &_execution);

    // reporter
    FileReporting _reporter("Reporting");
    NonRealTimeActivity reportingTask(0.2, _reporter.engine());
    connectPeers(&_reporter, &_execution);
    connectPeers(&_reporter, &_estimation);
    connectPeers(&_reporter, &_wrenchSensor);
    _reporter.load();
 
    // supervising TaskContext
    TaskContext _supervision("supervision");
    PeriodicActivity _supervisionTask(1,0.002,_supervision.engine());
    connectPeers(&_supervision, &_robot);
    connectPeers(&_supervision, &refsensorkuka160);
    connectPeers(&_supervision, &_wrenchSensor);
    connectPeers(&_supervision, &_nAxesSensor);
    connectPeers(&_supervision, &_nAxesGeneratorPos);
    connectPeers(&_supervision, &_nAxesGeneratorVel);
    connectPeers(&_supervision, &_nAxesControllerPos);
    connectPeers(&_supervision, &_nAxesControllerPosVel);
    connectPeers(&_supervision, &_nAxesControllerVel);
    connectPeers(&_supervision, &_nAxesEffectorVel);
    connectPeers(&_supervision, &_CartesianSensor);
    connectPeers(&_supervision, &_CartesianGeneratorPos);
    connectPeers(&_supervision, &_CartesianControllerPosVel);
    connectPeers(&_supervision, &_CartesianEffectorVel);
    connectPeers(&_supervision, &_execution);
    connectPeers(&_supervision, &_estimation);
    connectPeers(&_supervision, &_reporter);
    _supervision.scripting()->loadPrograms("cpf/program_calibrate_offsets.ops");
    _supervision.scripting()->loadPrograms("cpf/program_moveto.ops");
    _supervision.scripting()->loadStateMachines("cpf/states.osd");

    // emergency stop
    EmergencyStop emergency(&_robot);
    emergency.addEvent(&_robot, "driveOutOfRange");
    emergency.addEvent(&_robot, "positionOutOfRange");
    emergency.addEvent(&_wrenchSensor, "maximumLoadEvent");

    // start tasks
    _supervisionTask.start();

    // task browser
    TaskBrowser browser( &_supervision );
    browser.setColorTheme( TaskBrowser::whitebg );
    browser.loop();

    _supervisionTask.stop();
    return 0;
}
