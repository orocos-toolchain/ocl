
#include <rtt/RTT.hpp>
#include "DeploymentComponent.hpp"
#include <rtt/Activities.hpp>
#include <rtt/marsh/PropertyMarshaller.hpp>
#include <rtt/marsh/PropertyDemarshaller.hpp>

#include <cstdio>
#include <dlfcn.h>
#include "ocl/ComponentLoader.hpp"
#include <rtt/PropertyLoader.hpp>

#include <dirent.h>
#include <iostream>

using namespace Orocos;

namespace OCL
{
    using namespace std;
    using namespace RTT;

    DeploymentComponent::DeploymentComponent(std::string name)
        : RTT::TaskContext(name, Stopped),
          compPath("ComponentPath",
                   "Location to look for components in addition to the local directory and system paths.",
                   "/usr/local/orocos/lib"),
          autoUnload("AutoUnload",
                     "Stop and unload all components loaded by the DeploymentComponent when it is destroyed.",
                     true),
          validConfig("Valid", false),
          sched_RT("ORO_SCHED_RT", ORO_SCHED_RT ),
          sched_OTHER("ORO_SCHED_OTHER", ORO_SCHED_OTHER ),
          lowest_Priority("LowestPriority", RTT::OS::LowestPriority ),
          highest_Priority("HighestPriority", RTT::OS::HighestPriority )
    {
        this->properties()->addProperty( &compPath );
        this->properties()->addProperty( &autoUnload );

        this->attributes()->addAttribute( &validConfig );
        this->attributes()->addAttribute( &sched_RT );
        this->attributes()->addAttribute( &sched_OTHER );
        this->attributes()->addAttribute( &lowest_Priority );
        this->attributes()->addAttribute( &highest_Priority );


        this->methods()->addMethod( RTT::method("loadLibrary", &DeploymentComponent::loadLibrary, this),
                                    "Load a new library into memory.",
                                    "Name", "The name of the to be loaded library.");
        this->methods()->addMethod( RTT::method("import", &DeploymentComponent::import, this),
                                    "Load all libraries in Path into memory.",
                                    "Path", "The name of the directory where libraries are located.");


        this->methods()->addMethod( RTT::method("loadComponent", &DeploymentComponent::loadComponent, this),
                                    "Load a new component instance from a library.",
                                    "Name", "The name of the to be created component",
                                    "Type", "The component type, used to lookup the library.");
        this->methods()->addMethod( RTT::method("unloadComponent", &DeploymentComponent::unloadComponent, this),
                                    "Unload a loaded component instance.",
                                    "Name", "The name of the to be created component");
        this->methods()->addMethod( RTT::method("displayComponentTypes", &DeploymentComponent::displayComponentTypes, this),
                                    "Print out a list of all component types this component can create.");

        this->methods()->addMethod( RTT::method("loadConfiguration", &DeploymentComponent::loadConfiguration, this),
                                    "Load a new configuration.",
                                    "File", "The file which contains the new configuration.");
        this->methods()->addMethod( RTT::method("configureComponents", &DeploymentComponent::configureComponents, this),
                                    "Apply a loaded configuration.");
        this->methods()->addMethod( RTT::method("startComponents", &DeploymentComponent::startComponents, this),
                                    "Start the components configured for AutoStart.");
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
        if ( autoUnload.get() ) {
            // 1. Stop all activities, give components chance to cleanup.
            for ( CompList::iterator cit = comps.begin(); cit != comps.end(); ++cit) {
                ComponentData* it = &(cit->second);
                if ( it->loaded ) {
                    if ( it->instance->engine()->getActivity() == 0 || 
                         it->instance->engine()->getActivity()->isActive() == false ||
                         it->instance->stop() ) {
                        log(Info) << "Stopped "<< it->instance->getName() <<endlog();
                    } else {
                        log(Error) << "Could not stop loaded Component "<< it->instance->getName() <<endlog();
                    }
                }
            }
            // 2. Disconnect and destroy all components.
            for ( CompList::iterator cit = comps.begin(); cit != comps.end(); ++cit) {
                ComponentData* it = &(cit->second);
                std::string name = it->instance->getName();
		
                if ( it->loaded && it->instance ) {
                    if ( it->instance->engine()->getActivity() == 0 ||
                         it->instance->engine()->getActivity()->isActive() == false ) {
		      
                        log(Debug) << "Disconnecting " <<name <<endlog();
                        it->instance->disconnect();
                        log(Debug) << "Terminating " <<name <<endlog();
                        delete it->instance;
                        it->instance = 0;
                        delete it->act;
                        it->act = 0;
                        log(Info) << "Disconnected and destroyed "<< name <<endlog();
                    } else {
                        log(Error) << "Could not unload Component "<< name <<endlog();
                    }
                }
            }
        }
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

        PropertyBag from_file;
        log(Info) << "Loading '" <<configurationfile<<"'."<< endlog();
        // demarshalling failures:
        bool failure = false;  
        // semantic failures:
        bool valid = validConfig.get();
        PropertyDemarshaller demarshaller(configurationfile);
        try {
            if ( demarshaller.deserialize( from_file ) )
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
                            Property<std::string> importp = *it;
                            if ( !importp.ready() ) {
                                log(Error)<< "Found 'Import' statement, but it is not of type='string'."<<endlog();
                                valid = false;
                                continue;
                            }
                            if ( this->import( importp.get() ) == false )
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
                        if ( comp.value().find("Peers") != 0) {
                            Property<PropertyBag> nm = comp.value().getProperty<PropertyBag>("Peers");
                            if ( !nm.ready() ) {
                                log(Error)<<"Property 'Peers' must be a 'struct'."<<endlog();
                                valid = false;
                            } else {
                                for (PropertyBag::const_iterator it= nm.rvalue().begin(); it != nm.rvalue().end();it++) {
                                    Property<std::string> pr = *it;
                                    if ( !pr.ready() ) {
                                        log(Error)<<"Property 'Peer' does not have type 'string'."<<endlog();
                                        valid = false;
                                        continue;
                                    }
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

                        // Other options:
                        for (PropertyBag::const_iterator optit= comp.rvalue().begin(); optit != comp.rvalue().end();optit++) {
                            if ( (*optit)->getName() == "AutoStart" ) {
                                Property<bool> ps = comp.rvalue().getProperty<bool>("AutoStart");
                                if (!ps.ready()) {
                                    log(Error) << "AutoStart must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].autostart = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "AutoConf" ) {
                                Property<bool> ps = comp.rvalue().getProperty<bool>("AutoConf");
                                if (!ps.ready()) {
                                    log(Error) << "AutoConf must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].autoconf = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "ProgramScript" ) {
                                PropertyBase* ps = comp.rvalue().getProperty<string>("ProgramScript");
                                if (!ps) {
                                    log(Error) << "ProgramScript must be of type <string>" << endlog();
                                    valid = false;
                                }
                                continue;
                            }
                            if ( (*optit)->getName() == "StateMachineScript" ) {
                                PropertyBase* ps = comp.rvalue().getProperty<string>("StateMachineScript");
                                if (!ps) {
                                    log(Error) << "StateMachineScript must be of type <string>" << endlog();
                                    valid = false;
                                }
                                continue;
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
                log(Error)<< "Uncaught exception in deserialize !"<< endlog();
                failure = true;
            }
        return !failure && valid;
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

        // Connect peers
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

            // Setup the connections from each component to the
            // others.
            Property<PropertyBag> peers = comp.rvalue().getProperty<PropertyBag>("Peers");
            if ( peers.ready() )
                for (PropertyBag::const_iterator it= peers.rvalue().begin(); it != peers.rvalue().end();it++) {
                    Property<string> nm = (*it);
                    if ( nm.ready() )
                        this->addPeer( comp.getName(), nm.value() );
                    else {
                        log(Error) << "Wrong property type in Peers struct. Expected property of type 'string',"
                                   << " got type "<< (*it)->getType() <<endlog();
                        valid = false;
                    }
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

        // Main configuration
        for (PropertyBag::iterator it= root.begin(); it!=root.end();it++) {
            if ( (*it)->getName() == "Import" ) {
                continue;
            }
            
            Property<PropertyBag> comp = *it;

            TaskContext* peer = this->getPeer( comp.getName() );

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

            // Attach activities
            if ( comps[comp.getName()].act ) {
                log(Info) << "Setting activity of "<< comp.getName() <<endlog();
                comps[comp.getName()].act->run( peer->engine() );
                assert( peer->engine()->getActivity() == comps[comp.getName()].act );
            }

            // Load programs
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

            // AutoConf
            if (comps[comp.getName()].autoconf )
                if ( peer->configure() == false) 
                    valid = false;
        }

        validConfig.set(valid);
        return valid;
    }

    bool DeploymentComponent::startComponents()
    {
        Logger::In in("DeploymentComponent::startComponents");
        if (validConfig.get() == false) {
            log(Error) << "Not starting components with invalid configuration." <<endlog();
            return false;
        }
        bool valid = true;
        for (PropertyBag::iterator it= root.begin(); it!=root.end();it++) {
            if ( (*it)->getName() == "Import" )
                continue;
            
            TaskContext* peer = this->getPeer( (*it)->getName() );
            // AutoStart
            if (comps[(*it)->getName()].autostart )
                if ( peer->start() == false) 
                    valid = false;
        }
        return valid;
    }

    void DeploymentComponent::clearConfiguration()
    {
        log(Info) << "Clearing configuration options."<< endlog();
        conmap.clear();
        deletePropertyBag( root );
    }

    int filter_so(const struct dirent * d)
    {
        std::string so_name(d->d_name);
        // return FALSE if string ends in .so:
        if ( so_name.find("/.") != std::string::npos ||
             so_name.rfind("~") == so_name.length() -1 ) {
            log(Debug) << "Skipping " << so_name <<endlog(); 
            return 0;
        }
        return 1;
    }

    bool DeploymentComponent::import(const std::string& path)
    {
        Logger::In in("DeploymentComponent::import");
        struct dirent **namelist;
        int n;

        log(Info) << "Importing: " <<  path << endlog();

        // scan the directory for files.
        n = scandir( path.c_str(), &namelist, &filter_so, &alphasort);
        if (n < 0) {
            // path not found or 1 library specified.
            //perror("scandir");
            return loadLibrary( path );
        } else {
            int i = 0;
            while(i != n) {
                std::string name = namelist[i]->d_name;
                if (name != "." && name != ".." )
                    if (namelist[i]->d_type == DT_DIR) { //ignoring symlinks and subdirs here.
                        import( path +"/" +name );
                    } else {
                        log(Info) << "Scanning " << path +"/" +name <<endlog();
                        if ( name.rfind(".so") == std::string::npos ||
                             name.substr(name.rfind(".so")) != ".so" ) {
                            log(Debug) << "Dropping " << name <<endlog(); 
                        }
                        else {
                            log(Debug) << "Accepting " << name <<endlog(); 
                            loadLibrary( path + "/" + name );
                        }
                    }
                free(namelist[i]);
                ++i;
            }
            free(namelist);
        }
        return true;
    }

    bool DeploymentComponent::loadLibrary(const std::string& name)
    {
        Logger::In in("DeploymentComponent::loadLibrary");

        void *handle;

        // try dynamic loading:
        std::string so_name(name);
        if ( so_name.rfind(".so") == std::string::npos ||
             so_name.substr(so_name.rfind(".so")) != ".so" )
            so_name += ".so";

        handle = dlopen ( so_name.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!handle && so_name.find("/") == std::string::npos ) {
            so_name = "lib" + so_name;
            handle = dlopen ( so_name.c_str(), RTLD_NOW | RTLD_GLOBAL);
        }

        if (!handle) {
            log(Error) << "Could not load library '"<< name <<"':"<<endlog();
            log(Error) << dlerror() << endlog();
            return false;
        }

        dlerror();    /* Clear any existing error */
        
        // Lookup factories:
        FactoryMap* (*getfactory)(void) = 0;
        FactoryMap* fmap = 0;
        char* error = 0;
        getfactory = reinterpret_cast<FactoryMap*(*)(void)>( dlsym(handle, "getComponentFactoryMap") );
        if ((error = dlerror()) == NULL) {
            // symbol found, register factories...
            fmap = (*getfactory)();
            ComponentFactories::Instance().insert( fmap->begin(), fmap->end() );
            log(Info) << "Loaded component library '"<<so_name <<"'"<<endlog();
            return true;
        }

        // Lookup createComponent:
        dlerror();    /* Clear any existing error */

        TaskContext* (*factory)(std::string name) = 0;
        factory = reinterpret_cast<TaskContext*(*)(std::string name)>(dlsym(handle, "createComponent") );
        if ((error = dlerror()) == NULL) {
            // store factory.
            string libname = name.substr(name.rfind("/"), name.rfind(".so"));
            if ( libname.find("lib") == 0 )
                libname = libname.substr(3, std::string::npos); // strip leading lib if any.
            ComponentFactories::Instance()[libname] = factory;
            log(Info) << "Loaded component type '"<< libname <<"'"<<endlog();
            return true;
        }

        // plain library
        log(Info) << "Loaded library '"<< so_name <<"'"<<endlog();
        return true;
    }

    // or type is a shared library or it is a class type.
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

        // First: try loading from imported libraries. (see: import).
        factory = ComponentFactories::Instance()[ type ];

        if ( factory ) {
            log(Info) <<"Found factory for Component type "<<type<<endlog();
        } else {
            // Second: try dynamic loading:
            std::string so_name(type);
            if ( so_name.rfind(".so") == std::string::npos ||
                 so_name.substr(so_name.rfind(".so")) != ".so" )
                so_name += ".so";

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
            factory = reinterpret_cast<TaskContext*(*)(std::string)>(dlsym(handle, "createComponent"));
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

    void DeploymentComponent::displayComponentTypes() const\
    {
        OCL::FactoryMap::iterator it;
        cout << "I can create the following component types: " <<endl;
        for(it = OCL::ComponentFactories::Instance().begin(); it != OCL::ComponentFactories::Instance().end(); ++it) {
            cout << "   " << it->first << endl;
        }
        if ( OCL::ComponentFactories::Instance().size() == 0 )
            cout << "   (none)"<<endl;
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
        TaskContext* peer;
        if ( comp_name == this->getName() )
            peer = this;
        else
            peer = this->getPeer(comp_name);
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
            newact = new PeriodicActivity(scheduler, priority, period);
        else
            if ( act_type == "NonPeriodicActivity" && period == 0.0)
                newact = new NonPeriodicActivity(scheduler, priority);
            else
                if ( act_type == "SlaveActivity" )
                    newact = new SlaveActivity(period);
                
        if (newact == 0) {
            log(Error) << "Can't create activity for component "<<comp_name<<": incorrect arguments."<<endlog();
            return false;
        }

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
        TaskContext* c;
        if ( name == this->getName() )
            c = this;
        else
            c = this->getPeer(name);
        if (!c) {
            log(Error)<<"No such peer to configure: "<<name<<endlog();
            return false;
        }
            
        PropertyLoader pl;
        return pl.configure( filename, c, true ); // strict:true 
    }
}
