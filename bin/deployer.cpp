#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <deployment/DeploymentComponent.hpp>

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

    tb.loop();

    return 0;
}
