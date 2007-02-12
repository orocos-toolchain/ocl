
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
#include <rtt/PropertyLoader.hpp>


using namespace Orocos;

namespace OCL
{
    using namespace std;
    using namespace RTT;

    DeploymentComponent::DeploymentComponent(std::string name)
        : RTT::TaskContext(name),
          compPath("ComponentPath",
                   "Location to look for components in addition to the local directory and system paths.",
                   "/usr/local/orocos/lib"),
          validConfig("Valid", false),
          sched_RT("ORO_SCHED_RT", ORO_SCHED_RT ),
          sched_OTHER("ORO_SCHED_OTHER", ORO_SCHED_OTHER ),
          lowest_Priority("LowestPriority", RTT::OS::LowestPriority ),
          highest_Priority("HighestPriority", RTT::OS::HighestPriority )
    {
        this->attributes()->addAttribute( &validConfig );
        this->attributes()->addAttribute( &sched_RT );
        this->attributes()->addAttribute( &sched_OTHER );
        this->attributes()->addAttribute( &lowest_Priority );
        this->attributes()->addAttribute( &highest_Priority );


        this->methods()->addMethod( RTT::method("loadLibrary", &DeploymentComponent::loadLibrary, this),
                                    "Load a new library into memory.",
                                    "Name", "The name of the to be loaded library.");
        this->methods()->addMethod( RTT::method("import", &DeploymentComponent::loadLibrary, this),
                                    "Load a new library into memory.",
                                    "Name", "The name of the to be loaded library.");


        this->methods()->addMethod( RTT::method("loadComponent", &DeploymentComponent::loadComponent, this),
                                    "Load a new component instance from a library.",
                                    "Name", "The name of the to be created component",
                                    "Type", "The component type, used to lookup the library.");
        this->methods()->addMethod( RTT::method("unloadComponent", &DeploymentComponent::unloadComponent, this),
                                    "Unload a loaded component instance.",
                                    "Name", "The name of the to be created component");

        this->methods()->addMethod( RTT::method("loadConfiguration", &DeploymentComponent::loadConfiguration, this),
                                    "Load a new configuration.",
                                    "File", "The file which contains the new configuration.");
        this->methods()->addMethod( RTT::method("configureComponents", &DeploymentComponent::configureComponents, this),
                                    "Apply a loaded configuration.");
        // Work around compiler ambiguity:
        typedef bool(DeploymentComponent::*DCFun)(const std::string&, const std::string&);
        DCFun cp = &DeploymentComponent::connectPeers;
        this->methods()->addMethod( RTT::method("connectPeers", cp, this),
                                    "Connect two Components known to this Component.",
                                    "One", "The first component.","Two", "The second component.");
        cp = &DeploymentComponent::connectPorts;
        this->methods()->addMethod( RTT::method("connectPorts", cp, this),
                                    "Connect the Data Ports of two Components known to this Component.",
                                    "One", "The first component.","Two", "The second component.");
        cp = &DeploymentComponent::addPeer;
        typedef bool(DeploymentComponent::*DC4Fun)(const std::string&, const std::string&,
                                                   const std::string&, const std::string&);
        DC4Fun cp4 = &DeploymentComponent::connectPorts;
        this->methods()->addMethod( RTT::method("connectDataPorts", cp4, this),
                                    "Connect two data ports of Components known to this Component.",
                                    "One", "The first component.","Two", "The second component.",
                                    "PortOne", "The port name of the first component.",
                                    "PortTwo", "The port name of the second component.");

        this->methods()->addMethod( RTT::method("addPeer", cp, this),
                                    "Add a peer to a Component.",
                                    "From", "The first component.","To", "The other component.");
        typedef void(DeploymentComponent::*RPFun)(const std::string&);
        RPFun rp = &TaskContext::removePeer;
        this->methods()->addMethod( RTT::method("removePeer", rp, this),
                                    "Remove a peer from this Component.",
                                    "PeerName", "The name of the peer to remove.");

        this->methods()->addMethod( RTT::method("setPeriodicActivity", &DeploymentComponent::setPeriodicActivity, this),
                                    "Attach a periodic activity to a Component.",
                                    "CompName", "The name of the Component.",
                                    "Period", "The period of the activity.",
                                    "Priority", "The priority of the activity.",
                                    "SchedType", "The scheduler type of the activity."
                                    );
        this->methods()->addMethod( RTT::method("setNonPeriodicActivity", &DeploymentComponent::setNonPeriodicActivity, this),
                                    "Attach a non periodic activity to a Component.",
                                    "CompName", "The name of the Component.",
                                    "Priority", "The priority of the activity.",
                                    "SchedType", "The scheduler type of the activity."
                                    );
        this->methods()->addMethod( RTT::method("setSlaveActivity", &DeploymentComponent::setSlaveActivity, this),
                                    "Attach a slave activity to a Component.",
                                    "CompName", "The name of the Component.",
                                    "Period", "The period of the activity."
                                    );
        
        
    }
        
    DeploymentComponent::~DeploymentComponent()
    {
        RTT::deletePropertyBag(root);
        // Should we unload all loaded components here ?
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
        return t1->addPeer(t2);
    }

    bool DeploymentComponent::connectPorts(const std::string& one, const std::string& other)
    {
        TaskContext* a, *b;
        a = getPeer(one);
        b = getPeer(other);
        if ( !a ) {
            log(Error) << one <<" could not be found."<< endlog();
            return false;
        }
        if ( !b ) {
            log(Error) << other <<" could not be found."<< endlog();
            return false;
        }

        return a->connectPorts(b);
    }

    bool DeploymentComponent::connectPorts(const std::string& one, const std::string& one_port,
                                           const std::string& other, const std::string& other_port)
    {
        TaskContext* a, *b;
        a = getPeer(one);
        b = getPeer(other);
        if ( !a ) {
            log(Error) << one <<" could not be found."<< endlog();
            return false;
        }
        if ( !b ) {
            log(Error) << other <<" could not be found."<< endlog();
            return false;
        }
        PortInterface* ap, *bp;
        ap = a->ports()->getPort(one_port);
        bp = b->ports()->getPort(other_port);
        if ( !ap ) {
            log(Error) << one <<" does not have a port "<<one_port<< endlog();
            return false;
        }
        if ( !b ) {
            log(Error) << other <<" does not have a port "<<other_port<< endlog();
            return false;
        }

        // Detect already connected ports.
        if ( ap->connected() && bp->connected() ) {
            if (ap->connection() == bp->connection() ) {
                log(Info) << "Port '"<< ap->getName() << "' of Component '"<<a->getName()
                          << "' is already connected to port '"<< bp->getName() << "' of Component '"<<b->getName()<<"'."<<endlog();
                return true;
            }
            log(Error) << "Port '"<< ap->getName() << "' of Component '"<<a->getName()
                       << "' and port '"<< bp->getName() << "' of Component '"<<b->getName()
                       << "' are already connected but not to each other."<<endlog();
            return false;
        }

        // NOTE: all code below can be replaced by a single line:
        // bp->connectTo( ap ) || ap->connectTo(bp);
        // but that has less informational log messages.

        if ( ap->connected() ) {
            // ask peer to connect to us:
            if ( bp->connectTo( ap ) ) {
                log(Info)<< "Connected Port " << ap->getName()
                         << " of peer Task "<<ap->getName() << " to existing connection." << endlog();
                return true;
            }
            else {
                log(Error)<< "Failed to connect Port " << ap->getName()
                          << " of peer Task "<<ap->getName() << " to existing connection." << endlog();
                return false;
            }
        }

        // Peer port is connected thus our port is not connected.
        if ( bp->connected() ) {
            if ( ap->connectTo( bp ) ) {
                log(Info)<< "Added Port " << ap->getName()
                         << " to existing connection of Task "<<b->getName() << "." << endlog();
                return true;
            }
            else {
                log(Error)<< "Not connecting Port " << ap->getName()
                          << " to existing connection of Task "<<b->getName() << "." << endlog();
                return false;
            }
        }

        // Last resort: both not connected: create new connection.
        if ( !ap->connectTo( bp ) ) {
            // real error msg will be produced by factory itself.
            log(Warning)<< "Failed to connect Port " << ap->getName() << " of " << a->getName() << " to peer Task "<<b->getName() <<"." << endlog();
            return false;
        }
        
        // all went fine.
        log(Info)<< "Connected Port " << ap->getName() << " to peer Task "<< b->getName() <<"." << endlog();
        return true;
    }

    int string_to_oro_sched(std::string sched) {
        if ( sched == "ORO_SCHED_OTHER" )
            return ORO_SCHED_OTHER;
        if (sched == "ORO_SCHED_RT" )
            return ORO_SCHED_RT;
        log(Error)<<"Unknown scheduler type: "<< sched <<endlog();
        return -1;
    }

    bool DeploymentComponent::loadConfiguration(const std::string& configurationfile)
    {
        Logger::In in("DeploymentComponent::loadConfiguration");
#ifndef OROPKG_CORELIB_PROPERTIES_MARSHALLING
        log(Error) << "No Property Demarshaller configured !" << endlog();
        return false;
    
#else
        PropertyBag from_file;
        log(Info) << "Loading '" <<configurationfile<<"'."<< endlog();
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
            if ( demarshaller->deserialize( from_file ) )
                {
                    valid = true;
                    log(Info)<<"Validating new configuration..."<<endlog();
                    if ( from_file.empty() ) {
                        log(Error)<< "Configuration was empty !" <<endlog();
                        valid = false;
                    }

                    ConMap connections;
                    //for (PropertyBag::Names::iterator it= nams.begin();it != nams.end();it++) {
                    for (PropertyBag::iterator it= from_file.begin(); it!=from_file.end();it++) {
                        // Read in global options.
                        if ( (*it)->getName() == "Import" ) {
                            Property<std::string> import = *it;
                            if ( !import.ready() ) {
                                log(Error)<< "Found 'Import' statement, but it is not of type='string'."<<endlog();
                                valid = false;
                                continue;
                            }
                            if ( loadLibrary( import.get() ) == false )
                                valid = false;
                            continue;
                        }
                        // Check if it is a propertybag.
                        Property<PropertyBag> comp = *it;
                        if ( !comp.ready() ) {
                            log(Error)<< "Property '"<< *it <<"' is not a PropertyBag." << endlog();
                            valid = false;
                            continue;
                        }
                        // Check if we know this component.
                        TaskContext* c = this->getPeer( (*it)->getName() );
                        if ( !c ) {
                            // try to load it.
                            if (this->loadComponent( (*it)->getName(), comp.rvalue().getType() ) == false) {
                                log(Warning)<< "Could not configure '"<< (*it)->getName() <<"': No such peer."<< endlog();
                                valid = false;
                                continue;
                            }
                            c = this->getPeer( (*it)->getName() );
                        }

                        assert(c);

                        // set PropFile name if present
                        std::string filename = (*it)->getName() + ".cpf";
                        if ( comp.get().getProperty<std::string>("PropFile") )  // PropFile is deprecated
                            comp.get().getProperty<std::string>("PropFile")->setName("PropertyFile");

                        if ( comp.get().getProperty<std::string>("PropertyFile") )
                            if ( !comp.get().getProperty<std::string>("PropertyFile") ) {
                                valid = false;
                            }
                            
                        // connect ports 'Ports' tag is optional.
                        Property<PropertyBag>* ports = comp.get().getProperty<PropertyBag>("Ports");
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
                                    connections[ports->get().getProperty<std::string>(*pit)->get()].ports.push_back( p );
                                }
                            }
                        }

                        // Setup the connections from this
                        // component to the others.
                        for (PropertyBag::const_iterator it= comp.rvalue().begin(); it != comp.rvalue().end();it++) {
                            if ( (*it)->getName() == "Peer" ) {
                                Property<std::string> nm = *it;
                                if ( !nm.ready() ) {
                                    log(Error)<<"Property 'Peer' does not have type 'string'."<<endlog();
                                    valid = false;
                                    continue;
                                }
                            }
                        }
                        
                        // Read the activity profile if present.
                        if ( comp.value().find("Activity") != 0) {
                            Property<PropertyBag> nm = comp.value().getProperty<PropertyBag>("Activity");
                            if ( !nm.ready() ) {
                                log(Error)<<"Property 'Activity' must be a 'struct'."<<endlog();
                                valid = false;
                            } else {
                                if ( nm.rvalue().getType() == "PeriodicActivity" ) {
                                    Property<double> per;
                                    if (nm.rvalue().getProperty<double>("Period") )
                                        per = nm.rvalue().getProperty<double>("Period"); // work around RTT 1.0.2 bug.
                                    if ( !per.ready() ) {
                                        log(Error)<<"Please specify period <double> of PeriodicActivity."<<endlog();
                                        valid = false;
                                    }
                                    Property<int> prio;
                                    if ( nm.rvalue().getProperty<int>("Priority") )
                                        prio = nm.rvalue().getProperty<int>("Priority"); // work around RTT 1.0.2 bug
                                    if ( !prio.ready() ) {
                                        log(Error)<<"Please specify priority <short> of PeriodicActivity."<<endlog();
                                        valid = false;
                                    }
                                    Property<string> sched;
                                    if (nm.rvalue().getProperty<string>("Scheduler") )
                                        sched = nm.rvalue().getProperty<string>("Scheduler"); // work around RTT 1.0.2 bug
                                    int scheduler = ORO_SCHED_RT;
                                    if ( sched.ready() ) {
                                        scheduler = string_to_oro_sched( sched.get());
                                        if (scheduler == -1 )
                                            valid = false;
                                    }
                                    if (valid) {
                                        this->setActivity(comp.getName(), nm.rvalue().getType(), per.get(), prio.get(), scheduler );
                                    }
                                } else
                                    if ( nm.rvalue().getType() == "NonPeriodicActivity" ) {
                                        Property<int> prio;
                                        if ( nm.rvalue().getProperty<int>("Priority") )
                                            prio = nm.rvalue().getProperty<int>("Priority"); // work around RTT 1.0.2 bug
                                        if ( !prio ) {
                                            log(Error)<<"Please specify priority <short> of NonPeriodicActivity."<<endlog();
                                            valid = false;
                                        }
                                        Property<string> sched;
                                        if (nm.rvalue().getProperty<string>("Scheduler") )
                                            sched = nm.rvalue().getProperty<string>("Scheduler"); // work around RTT 1.0.2 bug
                                        int scheduler = ORO_SCHED_RT;
                                        if ( sched.ready() ) {
                                            int scheduler = string_to_oro_sched( sched.get());
                                            if (scheduler == -1 )
                                                valid = false;
                                        }
                                        if (valid) {
                                            this->setActivity(comp.getName(), nm.rvalue().getType(), 0.0, prio.get(), scheduler );
                                        }
                                    } else
                                        if ( nm.rvalue().getType() == "SlaveActivity" ) {
                                            double period = 0.0;
                                            if ( nm.rvalue().getProperty<double>("Period") )
                                                period = nm.rvalue().getProperty<double>("Period")->get();
                                            if (valid) {
                                                this->setActivity(comp.getName(), nm.rvalue().getType(), period, 0, 0 );
                                            }
                                        } else {
                                            log(Error) << "Unknown activity type: " << nm.rvalue().getType()<<endlog();
                                            valid = false;
                                        }
                            }
                        } else {
                            // no 'Activity' element, default to Slave:
                            this->setActivity(comp.getName(), "SlaveActivity", 0.0, 0, 0 );
                        }

                        // State machines and program scripts.
                        for (PropertyBag::const_iterator it= comp.rvalue().begin(); it != comp.rvalue().end();it++) {
                            if ( (*it)->getName() == "ProgramScript" ) {
                                PropertyBase* ps = comp.rvalue().getProperty<string>("ProgramScript");
                                if (!ps) {
                                    log(Error) << "ProgramScript must be of type <string>" << endlog();
                                    valid = false;
                                    continue;
                                }
                            }
                            if ( (*it)->getName() == "StateMachineScript" ) {
                                PropertyBase* ps = comp.rvalue().getProperty<string>("StateMachineScript");
                                if (!ps) {
                                    log(Error) << "StateMachineScript must be of type <string>" << endlog();
                                    valid = false;
                                    continue;
                                }
                            }
                        }

                    }
                        
                    if ( valid ) {
                        // put this config in the root config.
                        // existing component options are updated, new components are
                        // added to the back.
                        copyProperties( root, from_file );
                        conmap.insert( connections.begin(), connections.end() );
                    }
                    deletePropertyBag( from_file );
                }
            else
                {
                    log(Error)<< "Some error occured while parsing "<< configurationfile <<endlog();
                    failure = true;
                }
        } catch (...)
            {
                log(Error)<< "Uncaught exception in deserialise !"<< endlog();
                failure = true;
            }
        delete demarshaller;
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

        bool valid = true;

        // Load all property files into the components.
        for (PropertyBag::iterator it= root.begin(); it!=root.end();it++) {

            if ( (*it)->getName() == "Import" ) {
                continue;
            }
            
            Property<PropertyBag> comp = *it;

            TaskContext* peer = this->getPeer( comp.getName() );
            if ( !peer ) {
                log(Error) << "Peer not found: "<< comp.getName() <<endlog();
                valid=false;
                continue;
            }

            comps[comp.getName()].instance = peer;
                
            // set PropFile name if present
            std::string filename;
            if ( comp.get().getProperty<std::string>("PropertyFile") ){
                filename = comp.get().getProperty<std::string>("PropertyFile")->get();
            }
            if ( !filename.empty() ) {
                PropertyLoader pl;
                bool ret = pl.configure( filename, peer, true ); // strict:true 
                if (!ret) {
                    log(Error) << "Failed to configure properties for component "<< comp.getName() <<endlog();
                    valid = false;
                } else {
                    log(Info) << "Configured Properties of "<< comp.getName() <<endlog();
                }
            }

            Property<string> script;
            if (comp.get().getProperty<std::string>("ProgramScript") )
                script = comp.get().getProperty<std::string>("ProgramScript"); // work around RTT 1.0.2 bug
            if ( script.ready() ) {
                valid = valid && peer->scripting()->loadPrograms( script.get(), false );
            }
            if (comp.get().getProperty<std::string>("StateMachineScript") )
                script = comp.get().getProperty<std::string>("StateMachineScript");
            if ( script.ready() ) {
                valid = valid && peer->scripting()->loadStateMachines( script.get(), false );
            }

            // Attach activities
            if ( comps[comp.getName()].act ) {
                log(Info) << "Setting activity of "<< comp.getName() <<endlog();
                comps[comp.getName()].act->run( peer->engine() );
		assert( peer->engine()->getActivity() == comps[comp.getName()].act );
		
            }
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

        PropertyBag::Names nams = root.list();
        for (PropertyBag::Names::iterator nit= nams.begin(); nit!=nams.end();nit++) {
            if ( *nit == "Import" )
                continue;
            Property<PropertyBag> comp = root.getProperty<PropertyBag>(*nit);
            for (PropertyBag::const_iterator it= comp.rvalue().begin(); it != comp.rvalue().end();it++) {
                if((*it)->getName() == "Peer"){
                    Property<std::string> nm = *it;
                    this->addPeer( comp.getName(), nm.value() );
                }
            }
        }
            
        validConfig.set(valid);
        return valid;
    }

    void DeploymentComponent::clearConfiguration()
    {
        log(Info) << "Clearing configuration options."<< endlog();
        conmap.clear();
        deletePropertyBag( root );
    }

    bool DeploymentComponent::loadLibrary(const std::string& name)
    {
        Logger::In in("DeploymentComponent::loadLibrary");

        void *handle;

        // Second: try dynamic loading:
        std::string so_name = name +".so";

        handle = dlopen ( so_name.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            std::string error = dlerror();
            so_name = "lib" + so_name;
            handle = dlopen ( so_name.c_str(), RTLD_NOW | RTLD_GLOBAL);

            if (!handle) {
                log(Error) << "Could not load library '"<< name <<"':"<<endlog();
                log(Error) << error << endlog();
                log(Error) << dlerror() << endlog();
                return false;
            }
        }

        dlerror();    /* Clear any existing error */
        log(Info) << "Loaded library '"<< so_name <<"'"<<endlog();
        return true;
    }

    bool DeploymentComponent::loadComponent(const std::string& name, const std::string& type)
    {
        Logger::In in("DeploymentComponent::loadComponent");

        if ( this->getPeer(name) || comps.find(name) != comps.end() ) {
            log(Error) <<"Failed to load component with name "<<name<<": already present as peer or loaded."<<endlog();
            return false;
        }

        void *handle;
        TaskContext* (*factory)(std::string name) = 0;
        char *error;

        // First: try static loading:
        //factory = ComponentFactories::Factories[ type ];

        if ( factory ) {
            log(Info) <<"Found static factory for Component type "<<type<<endlog();
        } else {
            // Second: try dynamic loading:
            std::string so_name = type +".so";

            handle = dlopen ( so_name.c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (!handle) {
                std::string error = dlerror();
                // search in component path:
                handle = dlopen ( string(compPath.get() + "/" + so_name).c_str(), RTLD_NOW | RTLD_GLOBAL);
                if (!handle) {
                    log(Error) << "Could not load plugin '"<< so_name <<"':" <<endlog();
                    log(Error) << error << endlog();
                    log(Error) << dlerror() << endlog();
                    return false;
                }
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
        newcomp.loaded = true;

        if ( newcomp.instance == 0 ) {
            log(Error) <<"Failed to load component with name "<<name<<": refused to be created."<<endlog();
            return false;
        }

        log(Error) << "Adding "<< newcomp.instance->getName() << " as new peer: ";
        if (!this->addPeer( newcomp.instance ) ) {
            log() << "Failed !" << endlog(Error);
            delete newcomp.instance;
            return false;
        }
        log() << " OK."<< endlog(Info);
        comps[name] = newcomp;
        return true;
    }


    bool DeploymentComponent::unloadComponent(const std::string& name)
    {
        if ( comps[name].loaded == false ) {
            log(Error) << "Can't unload component "<<name<<": not loaded by "<<this->getName()<<endlog();
            comps.erase(name);
            return false;
        }
        TaskContext* peer = comps[name].instance;
        if ( peer->isRunning() ) {
            log(Error) << "Can't unload component "<<name<<" since it is still running."<<endlog();
            return false;
        }
        // todo: write properties ?
        peer->disconnect(); // if it is no longer a peer of this, that's ok.
        delete peer;
        comps.erase(name);
        log(Info) << "Succesfully unloaded component "<<name<<"."<<endlog();
        return true;
    }

    bool DeploymentComponent::setPeriodicActivity(const std::string& comp_name, 
                                                  double period, int priority,
                                                  int scheduler)
    {
        if ( this->setActivity(comp_name, "PeriodicActivity", period, priority, scheduler) ) {
            assert( comps[comp_name].instance );
            assert( comps[comp_name].act );
            comps[comp_name].act->run(comps[comp_name].instance->engine());
            return true;
        }
        return false;
    }
    
    bool DeploymentComponent::setNonPeriodicActivity(const std::string& comp_name, 
                                                     int priority,
                                                     int scheduler)
    {
        if ( this->setActivity(comp_name, "NonPeriodicActivity", 0.0, priority, scheduler) ) {
            assert( comps[comp_name].instance );
            assert( comps[comp_name].act );
            comps[comp_name].act->run(comps[comp_name].instance->engine());
            return true;
        }
        return false;
    }
    
    bool DeploymentComponent::setSlaveActivity(const std::string& comp_name, 
                                               double period)
    {
        if ( this->setActivity(comp_name, "SlaveActivity", period, 0, ORO_SCHED_OTHER ) ) {
            assert( comps[comp_name].instance );
            assert( comps[comp_name].act );
            comps[comp_name].act->run(comps[comp_name].instance->engine());
            return true;
        }
        return false;
    }
    

    bool DeploymentComponent::setActivity(const std::string& comp_name, 
                                          const std::string& act_type, 
                                          double period, int priority,
                                          int scheduler)
    {
        TaskContext* peer = this->getPeer(comp_name);
        if (!peer) {
            log(Error) << "Can't create Activity: component "<<comp_name<<" not found."<<endlog();
            return false;
        }
        comps[comp_name].instance = peer;
        if ( peer->isRunning() ) {
            log(Error) << "Can't change activity of component "<<comp_name<<" since it is still running."<<endlog();
            return false;
        }

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

#if 0
        if ( act_type != "SlaveActivity" ) {
            if (scheduler == ORO_SCHED_RT && newact->thread()->getScheduler() != ORO_SCHED_RT ) {
                newact->thread()->stop();
                newact->thread()->setScheduler(ORO_SCHED_RT);
                newact->thread()->start();
            }
                        
            if (scheduler == ORO_SCHED_OTHER && newact->thread()->getScheduler() != ORO_SCHED_OTHER ) {
                newact->thread()->stop();
                newact->thread()->setScheduler(ORO_SCHED_OTHER);
                newact->thread()->start();
            }
        }
#endif

        // this must never happen if component is running:
        assert( peer->isRunning() == false );
        delete comps[comp_name].act;
        comps[comp_name].act = newact;
                
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
