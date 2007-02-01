
#include <rtt/RTT.hpp>
#include "DeploymentComponent.hpp"
#include <rtt/Activities.hpp>
#ifndef OROCLS_CORELIB_PROPERTIES_DEMARSHALLING_DRIVER
#include <pkgconf/corelib_properties_marshalling.h>
#endif
#include ORODAT_CORELIB_PROPERTIES_MARSHALLING_INCLUDE
#include ORODAT_CORELIB_PROPERTIES_DEMARSHALLING_INCLUDE

#include <cstdio>
#include <dlfcn.h>
#include "ComponentLoader.hpp"


using namespace Orocos;

namespace OCL
{
    DeploymentComponent::DeploymentComponent(std::string name)
        : RTT::TaskContext(name),
          configurationfile("ConfigFile", "Name of the configuration file.", "cpf/"+name+".cpf"),
          autoConnect("AutoConnect", "Should available ports with the same name be auto-connected between peers ?", true),
          validConfig("Valid", false)
    {
        this->properties()->addProperty( &configurationfile );
        this->properties()->addProperty( &autoConnect );
        this->attributes()->addAttribute( &validConfig );

        this->methods()->addMethod( RTT::method("loadComponent", &DeploymentComponent::loadComponent, this),
                                    "Load a new component instance from a library.",
                                    "Name", "The name of the to be created component",
                                    "Type", "The component type, used to lookup the library.");
        this->methods()->addMethod( RTT::method("unloadComponent", &DeploymentComponent::unloadComponent, this),
                                    "Unload a loaded component instance.",
                                    "Name", "The name of the to be created component");

        this->methods()->addMethod( RTT::method("loadConfiguration", &DeploymentComponent::loadConfiguration, this),
                                    "Load and store the configuration file 'ConfigFille'.");
        this->methods()->addMethod( RTT::method("configureComponents", &DeploymentComponent::configureComponents, this),
                                    "Apply a loaded configuration.");
        // Work around compiler ambiguity:
        typedef bool(DeploymentComponent::*DCFun)(const std::string&, const std::string&);
        DCFun cp = &DeploymentComponent::connectPeers;
        this->methods()->addMethod( RTT::method("connectPeers", cp, this),
                                    "Connect two Components known to this Component.",
                                    "One", "The first component.","Two", "The second component.");
        cp = &DeploymentComponent::addPeer;
        this->methods()->addMethod( RTT::method("addPeer", cp, this),
                                    "Add a peer to a Component.",
                                    "From", "The first component.","To", "The other component.");
    }
        
    DeploymentComponent::~DeploymentComponent()
    {
        RTT::deletePropertyBag(root);
    }

    bool DeploymentComponent::connectPeers(const std::string& one, const std::string& other)
    {
        Logger::In in("DeploymentComponent");
        TaskContext* t1 = this->getPeer(one);
        TaskContext* t2 = this->getPeer(other);
        if (!t1) {
            log(Error)<< "No such peer: "<<one<<endlog();
            return false;
        }
        if (!t2) {
            log(Error) << "No such peer: "<<other<<endlog();
            return false;
        }
        if ( autoConnect.get() ) {
            log(Info) << "Automatically connecting data ports of "<<t1->getName() <<" and " << t2->getName() <<endlog();
            log(Info) << "Disable with AutoConnect = false."<< endlog();
            t1->connectPorts(t2);
        }
        return t1->connectPeers(t2);
    }

    bool DeploymentComponent::addPeer(const std::string& from, const std::string& to)
    {
        Logger::In in("DeploymentComponent");
        TaskContext* t1 = this->getPeer(from);
        TaskContext* t2 = this->getPeer(to);
        if (!t1) {
            log(Error)<< "No such peer: "<<from<<endlog();
            return false;
        }
        if (!t2) {
            log(Error)<< "No such peer: "<<to<<endlog();
            return false;
        }
        if ( autoConnect.get() ) {
            log(Info) << "Automatically connecting data ports of "<<t1->getName() <<" and " << t2->getName() <<endlog();
            log(Info) << "Disable with AutoConnect = false."<< endlog();
            t1->connectPorts(t2);
        }
        return t1->addPeer(t2);
    }

    bool DeploymentComponent::loadConfiguration()
    {
        Logger::In in("DeploymentComponent::loadConfiguration");
#ifndef OROPKG_CORELIB_PROPERTIES_MARSHALLING
        log(Error) << "No Property Demarshaller configured !" << endlog();
        return false;
    
#else
        deletePropertyBag(root);

        log(Info) << "Loading '" <<configurationfile.get()<<"'."<< endlog();
        // demarshalling failures:
        bool failure = false;  
        // semantic failures:
        bool valid = validConfig.get();
        OROCLS_CORELIB_PROPERTIES_DEMARSHALLING_DRIVER* demarshaller = 0;
        try
            {
                demarshaller = new OROCLS_CORELIB_PROPERTIES_DEMARSHALLING_DRIVER (configurationfile);
            } catch (...) {
                log(Error)<< "Could not open file "<< configurationfile << endlog();
                return false;
            }
        try {
            std::vector<CommandInterface*> assignComs;

            if ( demarshaller->deserialize( root ) )
                {
                    valid = true;
                    conmap.clear();
                    log(Info)<<"Validating new configuration..."<<endlog();
                    if ( root.empty() ) {
                        log(Error)<< "Configuration was empty !" <<endlog();
                        valid = false;
                    }
                    PropertyBag::Names nams = root.list();

                    for (PropertyBag::Names::iterator it= nams.begin();it != nams.end();it++) {
                        // Check if it is a propertybag.
                        Property<PropertyBag>* comp = root.getProperty<PropertyBag>(*it);
                        if ( comp == 0 ) {
                            log(Error)<< "Property '"<< *it <<"' is not a PropertyBag." << endlog();
                            valid = false;
                        }
                        // Check if we know this component.
                        TaskContext* c = this->getPeer( *it );
                        if ( !c ) {
                            log(Warning)<< "Could not configure '"<< *it <<"': No such peer."<< endlog();
                            valid = false;
                        }
                        // set PropFile name if present
                        std::string filename = *it + ".cpf";
                        if ( comp->get().getProperty<std::string>("PropFile") )  // PropFile is deprecated
                            if ( !comp->get().getProperty<std::string>("PropFile") ) {
                                valid = false;
                            }
                        if ( comp->get().getProperty<std::string>("PropertyFile") )
                            if ( !comp->get().getProperty<std::string>("PropertyFile") ) {
                                valid = false;
                            }
                            
                        // connect ports 'Ports' tag is optional.
                        Property<PropertyBag>* ports = comp->get().getProperty<PropertyBag>("Ports");
                        if ( ports != 0 ) {
                            PropertyBag::Names pnams = ports->get().list();
                            for (PropertyBag::Names::iterator pit= pnams.begin(); pit !=pnams.end(); pit++) {
                                PortInterface* p = c->ports()->getPort(*pit);
                                if ( !p ) {
                                    log(Error)<< "Component '"<< c->getName() <<"' does not have a Port '"<<*pit<<"'." << endlog();
                                    valid = false;
                                }
                                if ( ports->get().getProperty<std::string>(*pit) == 0) {
                                    log(Error)<< "Property '"<< *pit <<"' is not of type 'string'." << endlog();
                                    valid = false;
                                }
                                // store the port
                                if (valid) {
                                    log(Debug)<<"storing Port: "<<c->getName()<<"."<<p->getName();
                                    log(Debug)<<" in " << ports->get().getProperty<std::string>(*pit)->get() <<endlog();
                                    conmap[ports->get().getProperty<std::string>(*pit)->get()].ports.push_back( p );
                                }
                            }
                        }

                        // Setup the connections from this
                        // component to the others.
                        for (PropertyBag::iterator it= comp->value().begin(); it != comp->value().end();it++) {
                            if ( (*it)->getName() == "Peer" ) {
                                Property<std::string>* nm = Property<std::string>::narrow(*it);
                                if ( !nm ) {
                                    log(Error)<<"Property 'Peer' does not have type 'string'."<<endlog();
                                    valid = false;
                                }
                                if ( this->getPeer( nm->value() ) == 0 ) {
                                    log(Error)<<"No such Peer:"<<nm->value()<<endlog();
                                    valid = false;
                                }
                                delete nm; // narrow() returns a clone()
                            }
                        }
                    }
                        
                        
                    if ( !valid )
                        deletePropertyBag( root );
                }
            else
                {
                    log(Error)<< "Some error occured while parsing "<< configurationfile.rvalue() <<endlog();
                    failure = true;
                    deletePropertyBag( root );
                    conmap.clear();
                }
        } catch (...)
            {
                log(Error)<< "Uncaught exception in deserialise !"<< endlog();
                failure = true;
            }
        delete demarshaller;
        validConfig.set( valid );
        return !failure && valid;
#endif // OROPKG_CORELIB_PROPERTIES_MARSHALLING
    }

    /** 
     * Configure the components with a loaded configuration.
     * 
     * 
     * @return true if all components could be succesfully configured.
     */
    bool DeploymentComponent::configureComponents()
    {
        Logger::In in("DeploymentComponent::configureComponents");
        if ( root.empty() ) {
            Logger::log() << Logger::Error
                          << "No configuration loaded !" <<endlog();
            return false;
        }
        PropertyBag::Names nams = root.list();

        // Load all property files into the components.
        for (PropertyBag::Names::iterator it= nams.begin(); it!=nams.end();it++) {
            Property<PropertyBag>* comp = root.getProperty<PropertyBag>(*it);
                
            // set PropFile name if present
            std::string filename;
            if ( comp->get().getProperty<std::string>("PropFile") ){
                filename = comp->get().getProperty<std::string>("PropFile")->get();
	    }
            if ( comp->get().getProperty<std::string>("PropertyFile") ){
                filename = comp->get().getProperty<std::string>("PropertyFile")->get();
	    }
	    if ( !filename.empty() ) {
	      PropertyLoader pl;
	      TaskContext* peer = this->getPeer( *it );
	      assert( peer );
	      bool ret = pl.configure( filename, peer, true ); // strict:true 
	      if (!ret) {
		log(Error) << "Failed to configure properties for component "<<*it<<endlog();
	      } else {
		log(Info) << "Configured Properties of "<<*it<<endlog();
	      }
            }
                
            //peer->disconnect();
        }

        // Create data port connections:
        for(ConMap::iterator it = conmap.begin(); it != conmap.end(); ++it) {
            if ( it->second.ports.size() < 2 ){
                log(Error) << "Can not form connection "<<it->first<<" with only one Port."<< endlog();
                continue;
            }
            // first find a write and a read port.
            // This is quite complex since a 'ReadWritePort' can act as both.
            PortInterface* writer = 0, *reader = 0;
            ConnectionData::Ports::iterator p = it->second.ports.begin();
            while (p != it->second.ports.end() && (writer == 0 || reader == 0) ) {
                if ( (*p)->getPortType() == PortInterface::WritePort ) {
                    if (writer && writer->getPortType() == PortInterface::ReadWritePort )
                        reader = writer;
                    writer = (*p)->clone();
                }
                else
                    if ( (*p)->getPortType() == PortInterface::ReadPort ) {
                        if (reader && reader->getPortType() == PortInterface::ReadWritePort )
                            writer = reader;
                        reader = (*p)->clone();
                    }
                    else
                        if ( (*p)->getPortType() == PortInterface::ReadWritePort )
                            if (writer == 0) {
                                writer = (*p)->clone();
                            }
                            else {
                                reader = (*p)->clone();
                            }
                    
                ++p;
            }
            // Idea is: create a clone or anticlone of a port
            // use that one to setup the connection object
            // then dispose it again.
            if ( writer == 0 ) {
                log(Warning) << "Connecting only read-ports in connection " << it->first << endlog();
                // solve this issue by using a temporary anti-port.
                writer = it->second.ports.front()->antiClone();
            }
            if ( reader == 0 ) {
                log(Warning) << "Connecting only write-ports in connection " << it->first << endlog();
                // make an anticlone of a writer
                reader = it->second.ports.front()->antiClone();
            }
                
            log(Info) << "Creating Connection "<<it->first<<":"<<endlog();
            ConnectionInterface::shared_ptr con = writer->createConnection( reader );
            assert( con );
            con->connect();
            // connect all ports to connection
            p = it->second.ports.begin();
            while (p != it->second.ports.end() ) {
                if ((*p)->connectTo( con ) == false) {
                    log(Error) << "Could not connect Port "<< (*p)->getName() << " to connection " <<it->first<<endlog();
                    if ((*p)->connected())
                        log(Error) << "Port "<< (*p)->getName() << " already connected !"<<endlog();
                    else
                        log(Error) << "Port "<< (*p)->getName() << " has wrong type !"<<endlog();
                } else
                    log(Info) << "Connected Port "<< (*p)->getName() <<" to connection " << it->first <<endlog();
                ++p;
            }
            // writer,reader was a clone or anticlone.
            delete writer;
            delete reader;
        }

        // Setup the connections from each component to the
        // others.
        for (PropertyBag::Names::iterator nit= nams.begin(); nit!=nams.end();nit++) {
            Property<PropertyBag>* comp = root.getProperty<PropertyBag>(*nit);
            for (PropertyBag::iterator it= comp->value().begin(); it != comp->value().end();it++) {
                if((*it)->getName() == "Peer"){
                    Property<std::string>* nm = Property<std::string>::narrow(*it);
                    this->addPeer( comp->getName(), nm->value() );
                    delete nm; // was a clone.
                }
            }
        }
            
        return true;
    }

    bool DeploymentComponent::loadComponent(const std::string& name, const std::string& type)
    {
        Logger::In in("DeploymentComponent::loadComponent");

        CompList::iterator it = comps.begin();
        while (it != comps.end() ) {
            if (it->instance->getName() == name ) {
                log(Error) <<"Failed to load component with name "<<name<<": already loaded."<<endlog();
                return false;
            }
            ++it;
        }

        void *handle;
        TaskContext* (*factory)(std::string name);
        char *error;

        // First: try static loading:
        factory = ComponentFactories::Factories[ type ];

        if ( factory ) {
            log(Info) <<"Found static factory for Component type "<<type<<endlog();
        } else {
            // Second: try dynamic loading:
            std::string so_name = type +".so";

            handle = dlopen ( so_name.c_str(), RTLD_NOW);
            if (!handle) {
                log(Error) << "Could not find plugin '"<< so_name <<"':";
                log(Error) << dlerror() << endlog();
                return false;
            }

            dlerror();    /* Clear any existing error */
            *(void **) (&factory) = dlsym(handle, "createComponent");
            if ((error = dlerror()) != NULL) {
                log(Error) << "Found plugin '"<< so_name <<"', but it is not an Orocos Component:";
                log(Error) << error << endlog();
                dlclose( handle );
                return false;
            }
            log(Info) << "Found Orocos plugin '"<< so_name <<"'"<<endlog();
        }
        
        ComponentData newcomp;
        newcomp.instance = (*factory)(name);
        newcomp.type = type;

        if ( newcomp.instance == 0 ) {
            log(Error) <<"Failed to load component with name "<<name<<": refused to be created."<<endlog();
            return false;
        }

        comps.push_back( newcomp );
        log(Info) << "Adding "<< newcomp.instance->getName() << " as new Peer." <<endlog();
        this->addPeer( newcomp.instance );
        return true;
    }


    bool DeploymentComponent::unloadComponent(const std::string& name)
    {
        CompList::iterator it = comps.begin();
        while (it != comps.end() ) {
            if (it->instance->getName() == name ) {
                if ( it->instance->isRunning() ) {
                    log(Error) << "Can't unload component "<<name<<" since it is still running."<<endlog();
                    return false;
                }
                // todo: write properties ?
                it->instance->disconnect();
                delete it->instance;
                comps.erase(it);
                log(Info) << "Succesfully unloaded component "<<name<<"."<<endlog();
                return true;
            }
            ++it;
        }
        log(Error) << "Can't unload component "<<name<<": not loaded by "<<this->getName()<<endlog();
        return false;
    }

    bool DeploymentComponent::setActivity(const std::string& comp_name, 
                                          const std::string& act_type, 
                                          double period, int priority,
                                          const std::string& scheduler)
    {
        CompList::iterator it = comps.begin();
        while (it != comps.end() ) {
            if (it->instance->getName() == comp_name ) {
                if ( it->instance->isRunning() ) {
                    log(Error) << "Can't change activity of component "<<comp_name<<" since it is still running."<<endlog();
                    return false;
                }
                ActivityInterface* oldact = it->instance->engine()->getActivity();
                ActivityInterface* newact = 0;
                if ( act_type == "PeriodicActivity" && period != 0.0)
                    newact = new PeriodicActivity(priority, period);
                else
                    if ( act_type == "NonPeriodicActivity" && period == 0.0)
                        newact = new NonPeriodicActivity(priority);
                    else
                        if ( act_type == "SlaveActivity" )
                            newact = new SlaveActivity(period);
                
                if (newact == 0) {
                    log(Error) << "Can't create activity for component "<<comp_name<<": incorrect arguments."<<endlog();
                    return false;
                }

                it->instance->engine()->setActivity( newact );
                delete oldact;
                return true;
            }
            ++it;
        }
        log(Error) << "Can't create Activity for component "<<comp_name<<": not loaded by "<<this->getName()<<endlog();
        return false;
    }

    bool DeploymentComponent::configure(std::string name)
    {
        return configureFromFile( name,  name + ".cpf" );
    }

    bool DeploymentComponent::configureFromFile(std::string name, std::string filename)
    {
        Logger::In in("DeploymentComponent");
        TaskContext* c = this->getPeer(name);
        if (!c) {
            log(Error)<<"No such peer to configure: "<<name<<endlog();
            return false;
        }
            
        PropertyLoader pl;
        return pl.configure( filename, c, true ); // strict:true 
    }
}
