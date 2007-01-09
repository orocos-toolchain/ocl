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
#include <rtt/PeriodicActivity.hpp>

#include <rtt/os/main.h>

#include "hardware/laserdistance/LaserSensor.hpp"
//User interface
#include "taskbrowser/TaskBrowser.hpp"

//Reporting
#include "reporting/FileReporting.hpp"



using namespace std;
using namespace RTT;
using namespace Orocos;

/**
 * main() function
 */
int ORO_main(int arc, char* argv[])
{
    // Set log level more verbose than default,
    // such that we can see output :
    if ( Logger::log().getLogLevel() < Logger::Info ) {
        Logger::log().setLogLevel( Logger::Info );
        log(Info) << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<endlog();
    }

    LaserSensor laser("LaserSensor",2);
    
    PeriodicActivity laserTask(0,0.1, laser.engine() );
    FileReporting reporter("Reporting");

    reporter.addPeer(&laser);
    PeriodicActivity reportingTask(2,0.1, reporter.engine() );
 
    
    TaskBrowser browser( &laser );

    browser.loop();

    laserTask.stop();
    
    return 0;
}
