
#include "execution/TaskContext.hpp"


namespace Orocos
{

    /**
     * A Component for deploying (configuring) other components in an
     * application. It allows to create connections between components,
     * load the properties for components and rename the ports of components,
     * such that automatic port connection can take place.
     */
    class DeploymentComponent
        : public TaskContext
    {
        PropertyBag root;
        // STRUCTURE:
        //         Property<PropertyBag> component;
        //         //         Property<std::string> property-source;
        
        //         Property<PropertyBag> ports;
        //         //         Property<std::string> portname;
        
        //         Property<std::string> peername;

        Property<std::string> configurationfile;
        
    public:
        DeploymentComponent(std::string name = "Configurator")
            : TaskContext(name),
              configurationfile("ConfigFile", "Name of the configuration file.", name+".cpf")
        {
            this->properties()->addProperty( &configurationfile );
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
                Logger::log() <<Logger::Error<< "No such peer: "<<one<<Logger::endl;
                return false;
            }
            if (!t2) {
                Logger::log() <<Logger::Error<< "No such peer: "<<other<<Logger::endl;
                return false;
            }
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
                Logger::log() <<Logger::Error<< "No such peer: "<<from<<Logger::endl;
                return false;
            }
            if (!t2) {
                Logger::log() <<Logger::Error<< "No such peer: "<<to<<Logger::endl;
                return false;
            }
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
            Logger::log() <<Logger::Error << "No Property Demarshaller configured !" << Logger::endl;
            return false;
    
#else

            Logger::log() <<Logger::Info << "Loading '" <<configurationfile.get()"'."<< Logger::endl;
            bool failure = false;
            OROCLS_CORELIB_PROPERTIES_DEMARSHALLING_DRIVER* demarshaller = 0;
            try
                {
                    demarshaller = new OROCLS_CORELIB_PROPERTIES_DEMARSHALLING_DRIVER (configurationfile);
                } catch (...) {
                    Logger::log() << Logger::Error
                                  << "Could not open file "<< configurationfile << Logger::endl;
                    return false;
                }
            try {
                vector<CommandInterface*> assignComs;

                if ( demarshaller->deserialize( root ) )
                    {
                        bool valid = true;
                        Logger::Log() <<Logger::Info<<"Validating new configuration..."<<Logger::endl;
                        if ( root.empty() ) {
                            Logger::log() << Logger::Error
                                          << "Configuration was empty !" <<Logger::endl;
                            valid = false;
                        }
                        PropertyBag::Names nams = root.list();

                        for (PropertyBag::Names::iterator it= nams.begin(); nams.end()) {
                            // Check if it is a propertybag.
                            Property<PropertyBag>* comp = root.getProperty<PropertyBag>(*it);
                            if ( comp == 0 ) {
                                Logger::log() << Logger::Error
                                              << "Property '"<< *it <<"' is not a PropertyBag." << Logger::endl;
                                valid = false;
                            }
                            // Check if we know this component.
                            TaskContext* c = this->getPeer( *it );
                            if ( !c ) {
                                Logger::log() << Logger::Warning
                                              << "Could not configure '"<< *it <<"': No such peer."<< Logger::endl;
                                valid = false;
                            }
                            // set PropFile name if present
                            string filename = *it + ".cpf";
                            if ( comp->get()->getProperty("PropFile") )
                                if ( !comp->get()->getProperty<string>("PropFile") ) {
                                    valid = false;
                                }

                            // rename ports if necessary.
                            if ( comp->get()->getProperty("PropFile") ) {
                                Property<PropertyBag>* ports = comp->getProperty<PropertyBag>("Ports");
                                if ( ports != 0 ) {
                                    for (PropertyBag::Names::iterator pit= ports->begin(); ports->end()) {
                                        PortInterface* p = comp->ports()->getPort(*pit);
                                        if ( !p ) {
                                            Logger::log() << Logger::Error
                                                          << "Component '"<< comp->getName() <<"' does not have a Port '"<<*pit<<"'." << Logger::endl;
                                            return false;
                                        }
                                        if ( ports->getProperty<std::string>(*pit) == 0) {
                                            Logger::log() << Logger::Error
                                                          << "Property '"<< *pit <<"' is not of type 'string'." << Logger::endl;
                                            return false;
                                        }
                                        std::string pname = ports->getProperty<std::string>(*pit)->value();
                                    }
                                } else {
                                    valid = false;
                                }
                            }

                            // Setup the connections from this component to the others.
                            for (PropertyBag::Properties::iterator it= comp->begin(); comp->end()) {
                                if ( (*it)->getName() == "Peer" ) {
                                    Property<std::string>* nm = Property<std::string>::narrow(*it);
                                    if (nm)
                                        this->addPeer( comp->getName(), nm->value() );
                                    else {
                                        Logger::log()<<Logger::Error<<"Property 'Peer' does not have type 'string'."<<Logger::endl;
                                        return false;
                                    }
                                }
                            }
                
                        }
                    }
                else
                    {
                        Logger::log() << Logger::Error
                                      << "Some error occured while parsing "<< configurationfile.c_str() <<Logger::endl;
                        failure = true;
                        deleteProperties( root );
                    }
            } catch (...)
                {
                    Logger::log() << Logger::Error
                                  << "Uncaught exception in deserialise !"<< Logger::endl;
                    failure = true;
                }
            delete demarshaller;
            return !failure;
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
            if ( root.empty() ) {
                Logger::log() << Logger::Error
                              << "No configuration loaded !" <<Logger::endl;
                return false;
            }
            PropertyBag::Names nams = root.list();

            for (PropertyBag::Names::iterator it= nams.begin(); nams.end()) {
                // Check if it is a propertybag.
                Property<PropertyBag>* comp = root.getProperty<PropertyBag>(*it);
                if ( comp == 0 ) {
                    Logger::log() << Logger::Error
                                  << "Property '"<< *it <<"' is not a PropertyBag." << Logger::endl;
                    return false;
                }
                // Check if we know this component.
                TaskContext* c = this->getPeer( *it );
                if ( !c ) {
                    Logger::log() << Logger::Warning
                                  << "Could not configure '"<< *it <<"': No such peer."<< Logger::endl;
                    continue;
                }
                // set PropFile name if present
                string filename = *it + ".cpf";
                if ( comp->get()->getProperty<string>("PropFile") )
                    filename = comp->get()->getProperty<string>("PropFile")->get();

                PropertyLoader pl;
                bool ret = pl.configure( filename, c, true ); // strict:true 
                if (!ret)
                    continue;

                // rename ports if necessary.
                Property<PropertyBag>* ports = comp->getProperty<PropertyBag>("Ports");
                if ( ports != 0 ) {
                    for (PropertyBag::Names::iterator pit= ports->begin(); ports->end()) {
                        PortInterface* p = comp->ports()->getPort(*pit);
                        if ( !p ) {
                            Logger::log() << Logger::Error
                                          << "Component '"<< comp->getName() <<"' does not have a Port '"<<*pit<<"'." << Logger::endl;
                            return false;
                        }
                        if ( ports->getProperty<std::string>(*pit) == 0) {
                            Logger::log() << Logger::Error
                                          << "Property '"<< *pit <<"' is not of type 'string'." << Logger::endl;
                            return false;
                        }
                        std::string pname = ports->getProperty<std::string>(*pit)->value();
                        Logger::log() <<Logger::Info << "Renaming Port '" << p->getName()<< "' to '"<< pname<<"'." <<Logger::endl;
                        p->setName( pname );
                    }
                    Logger::log() <<Logger::Info << "Reconnecting '" << comp->getName()<< "'." <<Logger::endl;
                    // reconnect peer's ports.
                    c->reconnect();
                }

                // Setup the connections from this component to the others.
                for (PropertyBag::Properties::iterator it= comp->begin(); comp->end()) {
                    if ( (*it)->getName() == "Peer" ) {
                        Property<std::string>* nm = Property<std::string>::narrow(*it);
                        if (nm)
                            this->addPeer( comp->getName(), nm->value() );
                        else {
                            Logger::log()<<Logger::Error<<"Property 'Peer' does not have type 'string'."<<Logger::endl;
                            return false;
                        }
                    }
                }
                
            }
            return true;
        }

        bool configureComponent(std::string name)
        {
            TaskContext* c = this->getPeer(name);
            if (!c) {
                Logger::log()<<Logger::Error<<"No such peer to configure: "<<name<<Logger::endl;
                return false;
            }
            
            this->setProperties( c, name + ".cpf" );
        }

        bool startup()
        {}

        void update()
        {}

        void shutdown()
        {}
    };


}
