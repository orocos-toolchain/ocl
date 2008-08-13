/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:30:14 CEST 2008  deployer.cpp

                        deployer.cpp -  description
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
#include <deployment/DeploymentComponent.hpp>
#include <iostream>
#include <string>
#include "deployer-funcs.hpp"

namespace po = boost::program_options;

int ORO_main(int argc, char** argv)
{
	std::string             script;
	std::string             name("Deployer");
    po::variables_map       vm;

	int rc = OCL::deployerParseCmdLine(argc, argv, script, name, vm);
	if (0 != rc)
	{
		return rc;
	}

    OCL::DeploymentComponent dc( name );

    if ( !script.empty() )
        {
            dc.kickStart( script );
        }

    OCL::TaskBrowser tb( &dc );

    tb.loop();

    return 0;
}
