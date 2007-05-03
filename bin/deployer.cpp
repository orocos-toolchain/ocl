#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <deployment/DeploymentComponent.hpp>

#ifdef OROPKG_CORBA
#include <rtt/corba/ControlTaskServer.hpp>
#endif

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
    OCL::TaskBrowser tb( &dc );

    /**
     * If CORBA is enabled, export the DeploymentComponent
     * as CORBA server.
     */
#ifdef OROPKG_CORBA
    using namespace RTT::Corba;
    ControlTaskServer::InitOrb( argc, argv );

    ControlTaskServer::Create( &dc );

    ControlTaskServer::ThreadOrb();
#endif

    tb.loop();

#ifdef OROPKG_CORBA
    ControlTaskServer::ShutdownOrb();

    ControlTaskServer::DestroyOrb();
#endif
    return 0;
}
