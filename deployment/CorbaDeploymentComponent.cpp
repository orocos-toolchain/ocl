#include "CorbaDeploymentComponent.hpp"
#include <rtt/corba/ControlTaskProxy.hpp>
#include <rtt/corba/ControlTaskServer.hpp>
#include "ocl/ComponentLoader.hpp"
#include <fstream>

namespace OCL
{

    /**
     * This helper function looks up a server using the Naming Service
     * and creates a proxy for that object.
     */
    TaskContext* createControlTaskProxy(std::string name)
    {
        log(Debug) << "createControlTaskProxy" <<endlog();
        return ::RTT::Corba::ControlTaskProxy::Create(name, false);
    }
    
    /**
     * This helper function looks up a server using an IOR file
     * and creates a proxy for that object.
     */
    TaskContext* createControlTaskProxyIORFile(std::string iorfilename)
    {
        log(Debug) << "createControlTaskProxyIORFile" <<endlog();
        std::ifstream iorfile( iorfilename.c_str() );
        if (iorfile.is_open() && iorfile.good() ) {
            std::string ior;
            iorfile >> ior;
            return ::RTT::Corba::ControlTaskProxy::Create( ior, true);
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
    TaskContext* createControlTaskProxyIOR(std::string ior)
    {
        log(Debug) << "createControlTaskProxyIOR" <<endlog();
        return ::RTT::Corba::ControlTaskProxy::Create( ior, true);
    }
    

CorbaDeploymentComponent::CorbaDeploymentComponent(const std::string& name)
: DeploymentComponent(name)
{
    log(Info) << "Registering ControlTaskProxy factory." <<endlog();
    getFactories()["ControlTaskProxy"] = &createControlTaskProxy; 
    getFactories()["CORBA"] = &createControlTaskProxy; 
    getFactories()["IORFile"] = &createControlTaskProxyIORFile;
    getFactories()["IOR"] = &createControlTaskProxyIOR;
}

CorbaDeploymentComponent::~CorbaDeploymentComponent()
{
}

    bool CorbaDeploymentComponent::componentLoaded(TaskContext* c)
    {
        if ( dynamic_cast<RTT::Corba::ControlTaskProxy*>(c) ) {
            // is a proxy.
            return true;
        }
        bool use_naming = comps[c->getName()].use_naming;
        bool server = comps[c->getName()].server;
        log(Info) << "Name:"<< c->getName() << " Server: " << server << " Naming: " << use_naming <<endlog();
        // create a server, use naming.
        if (server)
            ::RTT::Corba::ControlTaskServer::Create(c, use_naming);
        return true;
    }
}
