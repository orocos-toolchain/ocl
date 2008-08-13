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
