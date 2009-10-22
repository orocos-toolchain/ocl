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
#include <rtt/os/main.h>
#include <kdl/bindings/rtt/toolkit.hpp>
#include "hardware/krypton/KryptonK600Sensor.hpp"
#include "taskbrowser/TaskBrowser.hpp"
#include "reporting/FileReporting.hpp"
#include <rtt/Toolkit.hpp>



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
      Logger::log().setLogLevel( Logger::Info );
      log(Info) << argv[0] << " manually raises LogLevel to 'Info' (5). "
		    << "See also file 'orocos.log'."<<endlog();
    }
    // import kdl toolkit
    Toolkit::Import( KDLToolkit );

    log(Info) << "creating krypton sensor" << endlog();
    KryptonK600Sensor krypton("KryptonSensor",6);

    log(Info) << "creating reporter" << endlog();
    FileReporting reporter("Reporting");
    reporter.connectPeers(&krypton);

    log(Info) << "start task browser" << endlog();
    TaskBrowser browser( &krypton );
    browser.loop();

    return 0;
}
