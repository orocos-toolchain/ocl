/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:34:48 CEST 2008  CorbaDeploymentComponent.cpp

                        CorbaDeploymentComponent.cpp -  description
                           -------------------
    begin                : Thu July 03 2008
    copyright            : (C) 2008 Peter Soetens
    email                : peter.soetens@fmtc.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/


#include "CorbaDeploymentComponent.hpp"
#include <rtt/transports/corba/ControlTaskProxy.hpp>
#include <rtt/transports/corba/ControlTaskServer.hpp>
#include <rtt/Method.hpp>
#include "ocl/ComponentLoader.hpp"
#include <fstream>

namespace OCL
{

    /**
     * This helper function looks up a server using the Naming Service
     * and creates a proxy for that object.
     */
    RTT::TaskContext* createControlTaskProxy(std::string name)
    {
        log(Debug) << "createControlTaskProxy" <<endlog();
        return ::RTT::corba::ControlTaskProxy::Create(name, false);
    }

    /**
     * This helper function looks up a server using an IOR file
     * and creates a proxy for that object.
     */
    RTT::TaskContext* createControlTaskProxyIORFile(std::string iorfilename)
    {
        log(Debug) << "createControlTaskProxyIORFile" <<endlog();
        std::ifstream iorfile( iorfilename.c_str() );
        if (iorfile.is_open() && iorfile.good() ) {
            std::string ior;
            iorfile >> ior;
            return ::RTT::corba::ControlTaskProxy::Create( ior, true);
        }
        else {
            log(Error) << "Could not open IORFile: '" << iorfilename <<"'."<< endlog();
            return 0;
        }
    }

    /**
     * This helper function looks up a server using an IOR file
     * and creates a proxy for that object.
     */
    RTT::TaskContext* createControlTaskProxyIOR(std::string ior)
    {
        log(Debug) << "createControlTaskProxyIOR" <<endlog();
        return ::RTT::corba::ControlTaskProxy::Create( ior, true);
    }


    CorbaDeploymentComponent::CorbaDeploymentComponent(const std::string& name)
        : DeploymentComponent(name)
    {
        log(Info) << "Registering ControlTaskProxy factory." <<endlog();
        getFactories()["ControlTaskProxy"] = &createControlTaskProxy;
        getFactories()["CORBA"] = &createControlTaskProxy;
        getFactories()["IORFile"] = &createControlTaskProxyIORFile;
        getFactories()["IOR"] = &createControlTaskProxyIOR;

        this->addOperation("server", &CorbaDeploymentComponent::createServer, this, ClientThread).doc("Creates a CORBA ControlTask server for the given component").arg("tc", "Name of the RTT::TaskContext (must be a peer).").arg("UseNamingService", "Set to true to use the naming service.");
    }

    CorbaDeploymentComponent::~CorbaDeploymentComponent()
    {
    }

    bool CorbaDeploymentComponent::createServer(const std::string& tc, bool use_naming)
    {
        RTT::TaskContext* peer = this->getPeer(tc);
        if (!peer) {
            log(Error)<<"No such peer: "<< tc <<endlog();
            return false;
        }
        if ( ::RTT::corba::ControlTaskServer::Create(peer, use_naming) != 0 )
            return true;
        return false;
    }


    bool CorbaDeploymentComponent::componentLoaded(RTT::TaskContext* c)
    {
        if ( dynamic_cast<RTT::corba::ControlTaskProxy*>(c) ) {
            // is a proxy.
            for ( CompList::iterator cit = comps.begin(); cit != comps.end(); ++cit) {
                if (cit->second.instance == c) {
                    cit->second.proxy = true;
                    return true;
                }
            }
            // impossible: proxy not found
            assert(false);
            return false;
        }
        bool use_naming = comps[c->getName()].use_naming;
        bool server = comps[c->getName()].server;
        log(Info) << "Name:"<< c->getName() << " Server: " << server << " Naming: " << use_naming <<endlog();
        // create a server, use naming.
        if (server)
            ::RTT::corba::ControlTaskServer::Create(c, use_naming);
        return true;
    }
}
