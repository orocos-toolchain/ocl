#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <deployment/CorbaDeploymentComponent.hpp>
#include <rtt/corba/ControlTaskServer.hpp>
#include <iostream>
#include "deployer-funcs.hpp"

using namespace std;
namespace po = boost::program_options;

int ORO_main(int argc, char** argv)
{
	std::string                 script;
	std::string                 name("Deployer");
	po::options_description     taoOptions("Additional options can also be passed to TAO");
	// we don't actually list any options for TAO ...

	int rc = deployerParseCmdLine(argc, argv, script, name, &taoOptions);
	if (0 != rc)
	{
		return rc;
	}

    OCL::CorbaDeploymentComponent dc( name );

    if ( !script.empty() )
	{
        dc.kickStart( script );
	}

    /**
     * If CORBA is enabled, export the DeploymentComponent
     * as CORBA server.
     */
    using namespace RTT::Corba;
    ControlTaskServer::InitOrb( argc, argv );

    ControlTaskServer::Create( &dc );

    ControlTaskServer::RunOrb();

    ControlTaskServer::ShutdownOrb();

    ControlTaskServer::DestroyOrb();

    return 0;
}
