
#include <rtt/os/main.h>
#include <rtt/corba/ControlTaskProxy.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <iostream>
#include <string>

int ORO_main(int argc, char** argv)
{
    if ( argc == 1) {
        cerr << "Please specify the CORBA ControlTask name or IOR to connect to." <<endl;
        cerr << "  " << argv[0] << " [ComponentName | IOR]" <<endl;
        return -1;
    }
    std::string name = argv[1];

    if ( name.substr(0, 4) == "IOR:" ) {
        RTT::Corba::ControlTaskProxy proxy( name );
        OCL::TaskBrowser tb( &proxy );
        tb.loop();
    } else {
        RTT::Corba::ControlTaskProxy proxy( name, true ); // is_ior = true
        OCL::TaskBrowser tb( &proxy );
        tb.loop();
    }

    return 0;
}
