/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:30:32 CEST 2008  cdeployer.cpp

                        cdeployer.cpp -  description
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
 *                                                                         *
 ***************************************************************************/


#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <deployment/CorbaDeploymentComponent.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <iostream>
#include "deployer-funcs.hpp"

using namespace std;
using namespace RTT::corba;
namespace po = boost::program_options;

int ORO_main(int argc, char** argv)
{
	std::string                 script;
	std::string                 name("Deployer");
    bool                        requireNameService = false;
    po::variables_map           vm;
	po::options_description     taoOptions("Additional options after a '--' are passed through to TAO");
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
                                       script, name, requireNameService,
                                       vm, &taoOptions);
	if (0 != rc)
	{
		return rc;
	}

	{
	    OCL::CorbaDeploymentComponent dc( name );

	    // if TAO options not found then have TAO process just the program name,
	    // otherwise TAO processes the program name plus all options (potentially
	    // none) after "--"
	    TaskContextServer::InitOrb( argc - taoIndex, &argv[taoIndex] );

	    if (0 == TaskContextServer::Create( &dc, true, requireNameService ))
	    {
	        return -1;
	    }

	    // Only start the script after the Orb was created.
	    if ( !script.empty() )
	    {
	        dc.kickStart( script );
	    }

	    // Export the DeploymentComponent as CORBA server.
	    TaskContextServer::RunOrb();
	}

    TaskContextServer::ShutdownOrb();

    TaskContextServer::DestroyOrb();

    return 0;
}
