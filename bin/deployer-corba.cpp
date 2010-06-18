/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:30:24 CEST 2008  deployer-corba.cpp

                        deployer-corba.cpp -  description
                           -------------------
    begin                : Thu July 03 2008
    copyright            : (C) 2008 Peter Soetens
    email                : peter.soetens@fmtc.be

 ***************************************************************************
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 ***************************************************************************/


#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <deployment/CorbaDeploymentComponent.hpp>
#include <rtt/corba/ControlTaskServer.hpp>
#include <iostream>
#include "deployer-funcs.hpp"

#include <rtt/corba/corba.h>

using namespace std;
using namespace OCL;
using namespace RTT;
using namespace RTT::Corba;
namespace po = boost::program_options;

int ORO_main(int argc, char** argv)
{
	std::string                 siteFile;      // "" means use default in DeploymentComponent.cpp
	std::string                 scriptFile;
	std::string                 name("Deployer");
    bool                        requireNameService = false;
	po::variables_map           vm;
	po::options_description     taoOptions("Additional options can also be passed to TAO");
	// we don't actually list any options for TAO ...

    // were we given TAO options? ie find "--"
    int     taoIndex    = 0;
    bool    found       = false;

    while(!found && taoIndex<argc)
    {
        found = (0 == strcmp("--", argv[taoIndex]));
        if(!found) taoIndex++;
    }

    if (found) {
        argv[taoIndex] = argv[0];
    }

    // if TAO options not found then process all command line options,
    // otherwise process all options up to but not including "--"
	int rc = OCL::deployerParseCmdLine(!found ? argc : taoIndex, argv,
                                       siteFile, scriptFile, name, requireNameService,
                                       vm, &taoOptions);
	if (0 != rc)
	{
		return rc;
	}

    try {
        // if TAO options not found then have TAO process just the program name,
        // otherwise TAO processes the program name plus all options (potentially
        // none) after "--"
        ControlTaskServer::InitOrb( argc - taoIndex, &argv[taoIndex] );

        {
            OCL::CorbaDeploymentComponent dc( name, siteFile );

            if (0 == ControlTaskServer::Create( &dc, true, requireNameService ))
                {
                    return -1;
                }

            // The orb thread accepts incomming CORBA calls.
            ControlTaskServer::ThreadOrb();

            // Only start the script after the Orb was created.
            if ( !scriptFile.empty() )
            {
                dc.kickStart( scriptFile );
            }

            OCL::TaskBrowser tb( &dc );

            tb.loop();
        }

        ControlTaskServer::ShutdownOrb();

        ControlTaskServer::DestroyOrb();

    } catch( CORBA::Exception &e ) {
        log(Error) << argv[0] <<" ORO_main : CORBA exception raised!" << Logger::nl;
        log() << CORBA_EXCEPTION_INFO(e) << endlog();
    }

    return 0;
}
