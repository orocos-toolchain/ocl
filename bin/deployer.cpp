#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <deployment/DeploymentComponent.hpp>
#include <iostream>
#include <string>
#include "deployer-funcs.hpp"

int ORO_main(int argc, char** argv)
{
	std::string             script;
	std::string             name("Deployer");

	int rc = deployerParseCmdLine(argc, argv, script, name);
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
