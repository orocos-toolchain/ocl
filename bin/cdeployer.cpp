#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <deployment/DeploymentComponent.hpp>
#include <rtt/corba/ControlTaskServer.hpp>

int ORO_main(int argc, char** argv)
{
    // deployer [DeployerName] [ -- TAO options ]
    char* name = "Deployer";
    if ( argc > 1 ) {
        std::string arg = argv[1];
        if ( arg != "--" )
            name = argv[1];
    }
    OCL::DeploymentComponent dc( name );

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
