
#include <rtt/os/main.h>
#include <rtt/corba/ControlTaskProxy.hpp>
#include <rtt/corba/ControlTaskServer.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <iostream>
#include <string>

using namespace RTT;
using namespace RTT::Corba;

int ORO_main(int argc, char** argv)
{
    if ( argc == 1) {
        std::cerr << "Please specify the CORBA ControlTask name or IOR to connect to." << std::endl;
        std::cerr << "  " << argv[0] << " [ComponentName | IOR]" << std::endl;
        return -1;
    }
    std::string name = argv[1];

    ControlTaskServer::InitOrb( argc, argv);

    ControlTaskServer::ThreadOrb();

    RTT::TaskContext* proxy;
    if ( name.substr(0, 4) == "IOR:" ) {
        proxy = RTT::Corba::ControlTaskProxy::Create( name, true );
    } else {
        proxy = RTT::Corba::ControlTaskProxy::Create( name ); // is_ior = true
    }

    if (proxy == 0){
        std::cerr << "CORBA system error while looking up " << name << std::endl;
        return -1;
    }
    OCL::TaskBrowser tb( proxy );
    tb.loop();

    ControlTaskServer::ShutdownOrb();
    ControlTaskServer::DestroyOrb();

    return 0;
}
