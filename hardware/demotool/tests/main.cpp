// Copyright (C) 2006 Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
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
#include <rtt/os/main.h>
#include <kdl/toolkit.hpp>
#include "hardware/demotool/Demotool.hpp"
#include "hardware/krypton/KryptonK600Sensor.hpp"
#include "hardware/wrench/WrenchSensor.hpp"
#include "taskbrowser/TaskBrowser.hpp"
#include "reporting/FileReporting.hpp"


using namespace std;
using namespace RTT;
using namespace KDL;
using namespace Orocos;


/**
 * main() function
 */
int ORO_main(int arc, char* argv[])
{
    // Set log level more verbose than default,
    // such that we can see output :
    if ( Logger::log().getLogLevel() < Logger::Info ) {
        Logger::log().setLogLevel( Logger::Debug );
        Logger::log() << Logger::Info << argv[0] << " manually raises LogLevel to 'Info' (5). "
		      << "See also file 'orocos.log'."<<Logger::endl;
    }
    // import kdl toolkit
    Toolkit::Import( KDLToolkit );

    // krypton
    KryptonK600Sensor krypton("Krypton",6);

    // wrench sensor
    WrenchSensor wrenchsensor(0.01, "Wrenchsensor", 0);
    NonPreemptibleActivity wrenchsensorTask(0.01, wrenchsensor.engine() );

    // demotool task
    Demotool demotool("Demotool");
    NonPreemptibleActivity demotoolTask(0.01, demotool.engine() );

    // reporter
    FileReporting reporter("Reporting");
    NonRealTimeActivity reporterTask(0.01, reporter.engine() );

    // connect tasks
    demotool.connectPeers(&krypton);
    demotool.connectPeers(&wrenchsensor);
    reporter.connectPeers(&demotool);
 
    // start tasks
    wrenchsensorTask.start();
    demotoolTask.start();

    // task browser
    reporter.load();
     TaskBrowser browser( &demotool );
    browser.loop();

    // stop tasks
    demotoolTask.stop();
    wrenchsensorTask.stop();
    reporterTask.stop();
    return 0;
}
