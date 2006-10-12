#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Logger.hpp>
#include <rtt/PropertyLoader.hpp>
#include <pkgconf/corelib_properties_marshalling.h>
#include <rtt/marsh/CPFDemarshaller.hpp>

namespace Orocos
{

    /**
     * A Component for deploying (configuring) other components in an
     * application. It allows to create connections between components and
     * load the properties for components,
     * such that connection of ports with different names can take place.
     */
    class DeploymentComponent
        : public TaskContext
    {
        PropertyBag root;
        // STRUCTURE:
        //         Property<PropertyBag> component;
        //         //         Property<std::string> property-source;
        
        //         Property<PropertyBag> ports;
        //         //         Property<std::string> portname; // value = connection_name
        
        //         Property<std::string> peername;

        Property<std::string> configurationfile;
        Attribute<bool> validConfig;

        struct ConnectionData {
            typedef std::vector<PortInterface*> Ports;
            Ports ports;
        };

        typedef std::map<std::string, ConnectionData> ConMap;
        ConMap conmap;
    public:
        DeploymentComponent(std::string name = "Configurator")
            : TaskContext(name),
              configurationfile("ConfigFile", "Name of the configuration file.", "cpf/"+name+".cpf"),
              validConfig("Valid", false)
        {
            this->properties()->addProperty( &configurationfile );
            this->attributes()->addAttribute( &validConfig );
        }

        /** 
         * Establish a bidirectional connection between two tasks.
         * 
         * @param one The first task to connect
         * @param other The second task to connect
         * 
         * @return true if both tasks are peers of this component and
         * could be connected.
         */
        bool connectPeers(const std::string& one, const std::string& other)
        {
            TaskContext* t1 = this->getPeer(one);
            TaskContext* t2 = this->getPeer(other);
            if (!t1) {
                log(Error)<< "No such peer: "<<one<<endlog();
                return false;
            }
            if (!t2) {
                Logger::log() <<Logger::Error<< "No such peer: "<<other<<endlog();
                return false;
            }
            t1->connectPorts(t2);
            return t1->connectPeers(t2);
        }
        /** 
         * Establish a uni directional connection form one task to another
         * 
         * @param from The component where the connection starts.
         * @param to The component where the connection ends.
         * 
         * @return true if both tasks are peers of this component and
         * a connection could be created.
         */
        bool addPeer(const std::string& from, const std::string& to)
        {
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
            t1->connectPorts(t2);
            return t1->addPeer(t2);
        }

        /** 
         * Load a configuration from disk. The 'ConfigFile' property is used to
         * locate the file. This does not apply the configuration yet on the
         * components.
         * @see configurePeers to configure the peer components with the loaded
         * configuration.
         * 
         * @return true if the configuration could be read and was valid.
         */
        bool loadConfiguration()
        {
            Logger::In in("DeploymentComponent::loadConfiguration");
#ifndef OROPKG_CORELIB_PROPERTIES_MARSHALLING
            log(Error) << "No Property Demarshaller configured !" << endlog();
            return false;
    
#else

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
                            
                            // connect ports.
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
                                    log(Debug)<<"storing Port: "<<c->getName()<<"."<<p->getName();
                                    log(Debug)<<" in " << ports->get().getProperty<std::string>(*pit)->get() <<endlog();
                                    conmap[ports->get().getProperty<std::string>(*pit)->get()].ports.push_back( p );
                                }
                            } else {
                                valid = false;
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
                                }
                            }
                        }
                        
                        
                        if ( !valid )
                            deleteProperties( root );
                    }
                else
                    {
                        log(Error)<< "Some error occured while parsing "<< configurationfile.rvalue() <<endlog();
                        failure = true;
                        deleteProperties( root );
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
        bool configureComponents()
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
                PortInterface* writer = 0, *reader = 0;
                ConnectionData::Ports::iterator p = it->second.ports.begin();
                while (p != it->second.ports.end() && (writer == 0 || reader == 0) ) {
                    if ( (*p)->getPortType() == PortInterface::WritePort )
                        writer = (*p)->clone();
                    else
                        if ( (*p)->getPortType() == PortInterface::ReadPort )
                            reader = (*p)->clone();
                        else
                            if ( (*p)->getPortType() == PortInterface::ReadWritePort )
                                writer = (*p)->clone();
                    
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
                log(Info) << "Connecting Port "<< writer->getName() <<" to Port " << reader->getName()<<endlog();
                ConnectionInterface::shared_ptr con = writer->createConnection( reader );
                // connect all ports to connection
                p = it->second.ports.begin();
                while (p != it->second.ports.end() ) {
                    if ((*p)->connectTo( reader ) == false) {
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

                //con->connect();
            }

            // Setup the connections from each component to the
            // others.
            for (PropertyBag::Names::iterator nit= nams.begin(); nit!=nams.end();nit++) {
                Property<PropertyBag>* comp = root.getProperty<PropertyBag>(*nit);
                for (PropertyBag::iterator it= comp->value().begin(); it != comp->value().end();it++) {
                    if((*it)->getName() == "Peer"){
                        Property<std::string>* nm = Property<std::string>::narrow(*it);
                        this->addPeer( comp->getName(), nm->value() );
                    }
                }
            }
            
            return true;
        }

        /**
         * Configure a component by loading the property file 'name.cpf' for component with
         * name \a name.
         * @param name The name of the component to configure.
         * The file used will be 'name.cpf'.
         * @return true if the component is known and the file could be
         * read.
         */
        bool configure(std::string name)
        {
            return configureFromFile( name,  name + ".cpf" );
        }

        /** 
         * Configure a component by loading a property file.
         * 
         * @param name The name of the component to configure
         * @param filename The filename where the configuration is in.
         * 
         * @return true if the component is known and the file could be
         * read.
         */
        bool configureFromFile(std::string name, std::string filename)
        {
            TaskContext* c = this->getPeer(name);
            if (!c) {
                log(Error)<<"No such peer to configure: "<<name<<endlog();
                return false;
            }
            
            PropertyLoader pl;
            return pl.configure( filename, c, true ); // strict:true 
        }

    };


}
