#include <rtt/os/main.h>
#include <taskbrowser/TaskBrowser.hpp>
#include <deployment/DeploymentComponent.hpp>

int ORO_main(int, char**)
{
    OCL::DeploymentComponent dc("Deployer");
    OCL::TaskBrowser tb( &dc );

    tb.loop();

    return 0;
}
