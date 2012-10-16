/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:30:32 CEST 2008  ctaskbrowser.cpp

                        ctaskbrowser.cpp -  description
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
 *                                                                         *
 ***************************************************************************/

#include <rtt/os/main.h>
#include <rtt/plugin/PluginLoader.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <iostream>
#include <string>

#include "installpath.hpp"

using namespace RTT;
using namespace RTT::corba;

#define ORO_xstr(s) ORO_str(s)
#define ORO_str(s) #s

int ORO_main(int argc, char** argv)
{
    
    if ( argc == 2 && strncmp(argv[1],"--version",9) == 0) {
        std::cout<< " OROCOS Toolchain version '" ORO_xstr(RTT_VERSION) "'";
#ifdef __GNUC__
        std::cout << " ( GCC " ORO_xstr(__GNUC__) "." ORO_xstr(__GNUC_MINOR__) "." ORO_xstr(__GNUC_PATCHLEVEL__) " )";
#endif
#ifdef OROPKG_OS_LXRT
        std::cout<<" -- LXRT/RTAI.";
#endif
#ifdef OROPKG_OS_GNULINUX
        std::cout<<" -- GNU/Linux.";
#endif
#ifdef OROPKG_OS_XENOMAI
        std::cout<<" -- Xenomai.";
#endif
        std::cout << std::endl;
        return 0;
    }

    if ( argc == 1 || (argc == 2 && strncmp(argv[1],"--help",6) == 0)) {
        std::cerr << "Please specify the CORBA TaskContext name or IOR to connect to." << std::endl;
        std::cerr << "  " << argv[0] << " [ComponentName | IOR]" << std::endl;
        return -1;
    }
    std::string name = argv[1];

    RTT::plugin::PluginLoader::Instance()->loadTypekits(ocl_install_path);

    TaskContextServer::InitOrb( argc, argv);

    TaskContextServer::ThreadOrb();

    RTT::TaskContext* proxy;
    if ( name.substr(0, 4) == "IOR:" ) {
        proxy = RTT::corba::TaskContextProxy::Create( name, true );
    } else {
        proxy = RTT::corba::TaskContextProxy::Create( name ); // is_ior = true
    }

    if (proxy == 0){
        std::cerr << "CORBA system error while looking up " << name << std::endl;
        return -1;
    }

    OCL::TaskBrowser tb( proxy );
    tb.loop();

    TaskContextServer::ShutdownOrb();
    TaskContextServer::DestroyOrb();

    return 0;
}
