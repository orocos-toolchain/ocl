
#include <rtt/RTT.hpp>
#include "DeploymentComponent.hpp"
#ifndef OROCLS_CORELIB_PROPERTIES_DEMARSHALLING_DRIVER
#include <pkgconf/corelib_properties_marshalling.h>
#endif
#include ORODAT_CORELIB_PROPERTIES_MARSHALLING_INCLUDE
#include ORODAT_CORELIB_PROPERTIES_DEMARSHALLING_INCLUDE

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
                        if ( comp->get().getProperty<std::string>("PropFile") )
                            if ( !comp->get().getProperty<std::string>("PropFile") ) {
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
            std::string filename = *it + ".cpf";
            if ( comp->get().getProperty<std::string>("PropFile") ){
                filename = comp->get().getProperty<std::string>("PropFile")->get();
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
