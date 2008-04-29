#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <deployment/CorbaDeploymentComponent.hpp>
#include <rtt/corba/ControlTaskServer.hpp>
#include <iostream>

using namespace std;
using namespace RTT;
using namespace RTT::Corba;

void usage(const char* name)
{
    cout << "usage: " << name << " [DeployerName] [--start config-file.xml]"<<endl;
    cout << endl;
}

int ORO_main(int argc, char** argv)
{
    // deployer --help
    // deployer [DeployerName] [--start config-file.xml]
    char* name = 0;
    char* script = 0;
    int i = 1;
    while ( i < argc && std::string(argv[i]) != "--" ) {
        std::string arg = argv[i];
        if ( arg == "--help" ) {
            usage(argv[0]);
            return 0;
        } else
        if ( arg == "--start" ) {
            ++i;
            if ( i < argc ) {
                script = argv[i];
            } else {
                cerr << "Please specify a filename after --start." <<endl;
                usage(argv[0]);
                return 0;
            }
        } else
            if (name == 0)
                name = argv[i];
            else {
                cerr << "Please specify only one name for the Deployer." <<endl;
                usage(argv[0]);
                return 0;
            }
        ++i;
    }

    if (name == 0)
        name = "Deployer";

    ControlTaskServer::InitOrb(argc, argv);
    
    OCL::CorbaDeploymentComponent dc( name );

    if (script)
        dc.kickStart( script );
    
    ControlTaskServer::InitOrb( argc, argv );

    ControlTaskServer::Create( &dc );

    // The orb thread accepts incomming CORBA calls.
    ControlTaskServer::ThreadOrb();

    OCL::TaskBrowser tb( &dc );

    tb.loop();

    ControlTaskServer::ShutdownOrb();

    ControlTaskServer::DestroyOrb();

    return 0;
}
