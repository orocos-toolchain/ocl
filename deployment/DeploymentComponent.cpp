        
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
#include <fstream>
#include <set>

using namespace Orocos;

namespace OCL
{
    using namespace std;
    using namespace RTT;

    std::vector<pair<string,void*> > DeploymentComponent::LoadedLibs;

    /**
     * I'm using a set to speed up lookups.
     */
    static std::set<string> valid_names;

#define ORO_str(s) ORO__str(s)
#define ORO__str(s) #s

    DeploymentComponent::DeploymentComponent(std::string name)
        : RTT::TaskContext(name, Stopped),
          compPath("ComponentPath",
                   "Location to look for components in addition to the local directory and system paths.",
                   "/usr/local/orocos/lib"),
          autoUnload("AutoUnload",
                     "Stop, cleanup and unload all components loaded by the DeploymentComponent when it is destroyed.",
                     true),
          validConfig("Valid", false),
          sched_RT("ORO_SCHED_RT", ORO_SCHED_RT ),
          sched_OTHER("ORO_SCHED_OTHER", ORO_SCHED_OTHER ),
          lowest_Priority("LowestPriority", RTT::OS::LowestPriority ),
          highest_Priority("HighestPriority", RTT::OS::HighestPriority ),
          target("Target",
                 "The Orocos Target component suffix. Will be used in import statements to find matching components. Only change this if you know what you are doing.",
                 ORO_str(OROCOS_TARGET) )
    {
        this->properties()->addProperty( &compPath );
        this->properties()->addProperty( &autoUnload );
        this->properties()->addProperty( &target );

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
                                    "Load a new XML configuration from a file (identical to loadComponents).",
                                    "File", "The file which contains the new configuration.");
        this->methods()->addMethod( RTT::method("loadConfigurationString", &DeploymentComponent::loadConfigurationString, this),
                                    "Load a new XML configuration from a string.",
                                    "Text", "The string which contains the new configuration.");
        this->methods()->addMethod( RTT::method("clearConfiguration", &DeploymentComponent::clearConfiguration, this),
                                    "Clear all configuration settings.");

        this->methods()->addMethod( RTT::method("loadComponents", &DeploymentComponent::loadComponents, this),
                                    "Load components listed in an XML configuration file.",
                                    "File", "The file which contains the new configuration.");
        this->methods()->addMethod( RTT::method("configureComponents", &DeploymentComponent::configureComponents, this),
                                    "Apply a loaded configuration to the components and configure() them if AutoConf is set.");
        this->methods()->addMethod( RTT::method("startComponents", &DeploymentComponent::startComponents, this),
                                    "Start the components configured for AutoStart.");
        this->methods()->addMethod( RTT::method("stopComponents", &DeploymentComponent::stopComponents, this),
                                    "Stop all the configured components (with or without AutoStart).");
        this->methods()->addMethod( RTT::method("cleanupComponents", &DeploymentComponent::cleanupComponents, this),
                                    "Cleanup all the configured components (with or without AutoConf).");
        this->methods()->addMethod( RTT::method("unloadComponents", &DeploymentComponent::unloadComponents, this),
                                    "Unload all the previously loaded components.");

        this->methods()->addMethod( RTT::method("kickStart", &DeploymentComponent::kickStart, this),
                                    "Calls loadComponents, configureComponents and startComponents in a row.",
                                    "File", "The file which contains the XML configuration to use.");
        this->methods()->addMethod( RTT::method("kickOut", &DeploymentComponent::kickOut, this),
                                    "Calls stopComponents, cleanupComponents and unloadComponents in a row.");

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
        

        this->configure();

        valid_names.insert("AutoUnload");
        valid_names.insert("UseNamingService");
        valid_names.insert("Server");
        valid_names.insert("AutoConf");
        valid_names.insert("AutoStart");
        valid_names.insert("AutoConnect");
        valid_names.insert("PropertyFile");
        valid_names.insert("UpdateProperties");
        valid_names.insert("ProgramScript");
        valid_names.insert("StateMachineScript");
        valid_names.insert("Ports");
        valid_names.insert("Peers");
        valid_names.insert("Activity");

    }

    bool DeploymentComponent::configureHook()
    {
        string toolpath = compPath.get() + "/rtt/"+ target.get() +"/plugins";
        DIR* res = opendir( toolpath.c_str() );
        if (res) {
            log(Info) << "Loading plugins from " + toolpath <<endlog();
            import( toolpath );
        } else {
            log(Info) << "No plugins present in " + toolpath << endlog();
        }
        return true;
    }

    bool DeploymentComponent::componentLoaded(TaskContext* c) { return true; }

    DeploymentComponent::~DeploymentComponent()
    {
      clearConfiguration();
      // Should we unload all loaded components here ?
      if ( autoUnload.get() ) {
          kickOut();
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
        if ( ap->ready() && bp->ready() ) {
            if (ap->connection() == bp->connection() ) {
                log(Info) << "Port '"<< ap->getName() << "' of Component '"<<a->getName()
                          << "' is already connected to port '"<< bp->getName() << "' of Component '"<<b->getName()<<"'."<<endlog();
                return true;
            }
            log(Error) << "Port '"<< ap->getName() << "' of Component '"<<a->getName()
                       << "' and port '"<< bp->getName() << "' of Component '"<<b->getName()
                       << "' are already connected but (probably) not to each other."<<endlog();
            return false;
        }

        // use the PortInterface implementation
        if ( ap->connectTo( bp ) ) {
            // all went fine.
            log(Info)<< "Connected Port " << ap->getName() << " to peer Task "<< b->getName() <<"." << endlog();
            return true;
        } else {
            log(Error)<< "Failed to connect Port " << ap->getName() << " to peer Task "<< b->getName() <<"." << endlog();
            return true;
        }
    }

    int string_to_oro_sched(const std::string& sched) {
        if ( sched == "ORO_SCHED_OTHER" )
            return ORO_SCHED_OTHER;
        if (sched == "ORO_SCHED_RT" )
            return ORO_SCHED_RT;
        log(Error)<<"Unknown scheduler type: "<< sched <<endlog();
        return -1;
    }

    bool DeploymentComponent::loadConfigurationString(const std::string& text)
    {
        const char* tmpfile = ".loadConfigurationString.cpf";
        std::ofstream file( tmpfile );
        file << text.c_str();
        file.close();
        return this->loadConfiguration( tmpfile );
    }

    bool DeploymentComponent::kickStart(const std::string& configurationfile)
    {
        if ( this->loadComponents(configurationfile) ) {
            if (this->configureComponents() ) {
                if ( this->startComponents() ) {
                    log(Info) <<"Successfully loaded, configured and started components from "<< configurationfile <<endlog();
                    return true;
                } else {
                    log(Error) <<"Failed to start a component: aborting kick-start."<<endlog();
                }
            } else {
                log(Error) <<"Failed to configure a component: aborting kick-start."<<endlog();
            }
        } else {
            log(Error) <<"Failed to load a component: aborting kick-start."<<endlog();
        }
        return false;
    }

    bool DeploymentComponent::kickOut()
    {
        bool sret = this->stopComponents();
        bool cret = this->cleanupComponents();
        bool uret = this->unloadComponents();
        if ( sret && cret && uret) {
            log(Info) << "Kick-out successful."<<endlog();
            return true;
        }
        // Diagnostics:
        log(Critical) << "Kick-out failed: ";
        if (!sret)
            log(Critical) << " stopComponents() failed.";
        if (!cret)
            log(Critical) << " cleanupComponents() failed.";
        if (!uret)
            log(Critical) << " unloadComponents() failed.";
        log(Critical) << endlog();
        return false;
    }

    bool DeploymentComponent::loadConfiguration(const std::string& configurationfile)
    {
        return this->loadComponents(configurationfile);
    }

    bool DeploymentComponent::loadComponents(const std::string& configurationfile)
    {
        Logger::In in("DeploymentComponent::loadComponents");

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
                        if ( (*it)->getName() == "Include" ) {
                            Property<std::string> includep = *it;
                            if ( !includep.ready() ) {
                                log(Error)<< "Found 'Include' statement, but it is not of type='string'."<<endlog();
                                valid = false;
                                continue;
                            }
                            // recursively call this function.
                            if ( this->loadComponents( includep.get() ) == false )
                                valid = false;
                            continue;
                        }
                        // Check if it is a propertybag.
                        Property<PropertyBag> comp = *it;
                        if ( !comp.ready() ) {
                            log(Error)<< "Property '"<< *it <<"' is should be a struct, Include or Import statement." << endlog();
                            valid = false;
                            continue;
                        }
                        // Parse the options before creating the component:
                        for (PropertyBag::const_iterator optit= comp.rvalue().begin(); optit != comp.rvalue().end();optit++) {
                            if ( valid_names.find( (*optit)->getName() ) == valid_names.end() ) {
                                log(Error) << "Unknown type syntax: '"<< (*optit)->getName() << "' in component struct "<< comp.getName() <<endlog();
                                valid = false;
                                continue;
                            }
                            if ( (*optit)->getName() == "AutoConnect" ) {
                                Property<bool> ps = comp.rvalue().getProperty<bool>("AutoConnect");
                                if (!ps.ready()) {
                                    log(Error) << "AutoConnect must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].autoconnect = ps.get();
                                continue;
                            }
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
                            if ( (*optit)->getName() == "Server" ) {
                                Property<bool> ps = comp.rvalue().getProperty<bool>("Server");
                                if (!ps.ready()) {
                                    log(Error) << "Server must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].server = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "UseNamingService" ) {
                                Property<bool> ps = comp.rvalue().getProperty<bool>("UseNamingService");
                                if (!ps.ready()) {
                                    log(Error) << "UseNamingService must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].use_naming = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "PropertyFile" ) {
                                PropertyBase* ps = comp.rvalue().getProperty<string>("PropertyFile");
                                if (!ps) {
                                    log(Error) << "PropertyFile must be of type <string>" << endlog();
                                    valid = false;
                                }
                                continue;
                            }
                            if ( (*optit)->getName() == "UpdateProperties" ) {
                                PropertyBase* ps = comp.rvalue().getProperty<string>("UpdateProperties");
                                if (!ps) {
                                    log(Error) << "UpdateProperties must be of type <string>" << endlog();
                                    valid = false;
                                }
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

                        // Check if we know this component.
                        TaskContext* c = this->getPeer( (*it)->getName() );
                        if ( !c ) {
                            // try to load it.
                            if (this->loadComponent( (*it)->getName(), comp.rvalue().getType() ) == false) {
                                log(Warning)<< "Could not configure '"<< (*it)->getName() <<"': No such peer."<< endlog();
                                valid = false;
                                continue;
                            }
                            c = comps[(*it)->getName()].instance;
                        } else {
                            // If the user added c as a peer (outside of Deployer) store the pointer
                            comps[(*it)->getName()].instance = c;
                        }

                        assert(c);

                        // set PropFile name if present
                        if ( comp.get().getProperty<std::string>("PropFile") )  // PropFile is deprecated
                            comp.get().getProperty<std::string>("PropFile")->setName("PropertyFile");

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
                                    conmap[ports->get().getProperty<std::string>(*pit)->get()].ports.push_back( p );
                                }
                            }
                        }

                        // Setup the connections from this
                        // component to the others.
                        if ( comp.value().find("Peers") != 0) {
                            Property<PropertyBag> nm = comp.value().find("Peers");
                            if ( !nm.ready() ) {
                                log(Error)<<"Property 'Peers' must be a 'struct', was type "<< comp.value().find("Peers")->getType() << endlog();
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
                            Property<PropertyBag> nm = comp.value().find("Activity");
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
                                        if ( !prio.ready() ) {
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
                            //this->setActivity(comp.getName(), "SlaveActivity", 0.0, 0, 0 );
                        }

                        // put this component in the root config.
                        // existing component options are updated, new components are
                        // added to the back.
                        // great: a hack to allow 'CompName.ior' as property name.
                        string delimiter("@!#?<!");
                        bool ret = updateProperty( root, from_file, comp.getName(), delimiter );
                        if (!ret) {
                            log(Error) << "Failed to store deployment properties for component " << comp.getName() <<endlog();
                            valid = false;
                        }
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
                log(Error)<< "Uncaught exception in loadcomponents() !"<< endlog();
                failure = true;
            }
        validConfig.set(valid);
        return !failure && valid;
    }

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
            
            Property<PropertyBag> comp = *it;

            TaskContext* peer = comps[ comp.getName() ].instance;
            if ( !peer ) {
                log(Error) << "Peer not found: "<< comp.getName() <<endlog();
                valid=false;
                continue;
            }

            comps[comp.getName()].instance = peer;

            // Setup the connections from each component to the
            // others.
            Property<PropertyBag> peers = comp.rvalue().find("Peers");
            if ( peers.ready() )
                for (PropertyBag::const_iterator it= peers.rvalue().begin(); it != peers.rvalue().end();it++) {
                    Property<string> nm = (*it);
                    if ( nm.ready() )
                        this->addPeer( comps[comp.getName()].instance->getName(), nm.value() );
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
                log(Warning) << "Can not form connection "<<it->first<<" with only one Port."<< endlog();
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
                    writer = (*p);
                }
                else
                    if ( (*p)->getPortType() == PortInterface::ReadPort ) {
                        if (reader && reader->getPortType() == PortInterface::ReadWritePort )
                            writer = reader;
                        reader = (*p);
                    }
                    else
                        if ( (*p)->getPortType() == PortInterface::ReadWritePort )
                            if (writer == 0) {
                                writer = (*p);
                            }
                            else {
                                reader = (*p);
                            }
                ++p;
            }
            // Inform the user of non-optimal connections:
            if ( writer == 0 ) {
                log(Warning) << "Connecting only read-ports in connection " << it->first << endlog();
                // let connectTo() below figure it out.
                writer = reader;
            }
            if ( reader == 0 ) {
                log(Warning) << "Connecting only write-ports in connection " << it->first << endlog();
            }
                
            log(Info) << "Creating Connection "<<it->first<<":"<<endlog();
            // connect all ports to connection
            p = it->second.ports.begin();
            while (p != it->second.ports.end() ) {
                // connect all readers to the first found writer.
                if ( *p != writer ) {
                    if ( (*p)->connectTo( writer ) == false) {
                        log(Error) << "Could not connect Port "<< (*p)->getName() << " to connection " <<it->first<<endlog();
                        if ((*p)->connected())
                            log(Error) << "Port "<< (*p)->getName() << " already connected !"<<endlog();
                        else
                            log(Error) << "Port "<< (*p)->getName() << " has wrong type !"<<endlog();
                        valid = false;
                    } else
                        log(Info) << "Connected Port "<< (*p)->getName() <<" to connection " << it->first <<endlog();
                }
                ++p;
            }
        }

        // Autoconnect ports.
        for (PropertyBag::iterator it= root.begin(); it!=root.end();it++) {
            Property<PropertyBag> comp = *it;
            if ( !comp.ready() )
                continue;

            TaskContext* peer = comps[ comp.getName() ].instance;

            // only autoconnect if AutoConnect == 1 and peer has AutoConnect == 1
            if ( comps[comp.getName()].autoconnect ) {
                TaskContext::PeerList peers = peer->getPeerList();
                for(TaskContext::PeerList::iterator pit = peers.begin(); pit != peers.end(); ++pit) {
                    if ( comps.count( *pit ) && comps[ *pit ].autoconnect ) {
                        TaskContext* other = peer->getPeer( *pit );
                        ::RTT::connectPorts( peer, other );
                    }
                }
            }
        }

        // Main configuration
        for (PropertyBag::iterator it= root.begin(); it!=root.end();it++) {
            
            Property<PropertyBag> comp = *it;
            Property<string> dummy;
            TaskContext* peer = comps[ comp.getName() ].instance;

            // Iterate over all elements
            for (PropertyBag::const_iterator pf = comp.rvalue().begin(); pf!= comp.rvalue().end(); ++pf) {
                // set PropFile name if present
                if ( (*pf)->getName() == "PropertyFile" || (*pf)->getName() == "UpdateProperties" ){
                    dummy = *pf; // convert to type.
                    string filename = dummy.get();
                    PropertyLoader pl;
                    bool strict = (*pf)->getName() == "PropertyFile" ? true : false;
                    bool ret = pl.configure( filename, peer, strict );
                    if (!ret) {
                        log(Error) << "Failed to configure properties for component "<< comp.getName() <<endlog();
                        valid = false;
                    } else {
                        log(Info) << "Configured Properties of "<< comp.getName() <<endlog();
                    }
                }
            }

            // Attach activities
            if ( comps[comp.getName()].act ) {
                if ( peer->engine()->getActivity() ) {
                    peer->engine()->getActivity()->run(0);
                    log(Info) << "Re-setting activity of "<< comp.getName() <<endlog();
                } else {
                    log(Info) << "Setting activity of "<< comp.getName() <<endlog();
                }
                comps[comp.getName()].act->run( peer->engine() );
                assert( peer->engine()->getActivity() == comps[comp.getName()].act );
            }

            // Load scripts in order of appearance
            for (PropertyBag::const_iterator ps = comp.rvalue().begin(); ps!= comp.rvalue().end(); ++ps) {
                Property<string> pscript;
                if ( (*ps)->getName() == "ProgramScript" )
                    pscript = *ps;
                if ( pscript.ready() ) {
                    valid = valid && peer->scripting()->loadPrograms( pscript.get(), false ); // no exceptions
                }
                Property<string> sscript;
                if ( (*ps)->getName() == "StateMachineScript" )
                    sscript = *ps;
                if ( sscript.ready() ) {
                    valid = valid && peer->scripting()->loadStateMachines( sscript.get(), false ); // no exceptions
                }
            }

            // AutoConf
            if (comps[comp.getName()].autoconf )
                if ( peer->configure() == false) 
                    valid = false;
        }

        // Finally, report success/failure:
        if (!valid) {
            for ( CompList::iterator cit = comps.begin(); cit != comps.end(); ++cit) {
                ComponentData* cd = &(cit->second);
                if ( cd->loaded && cd->autoconf && cd->instance->getTaskState() != TaskCore::Stopped )
                    log(Error) << "Failed to configure component "<< cd->instance->getName() <<endlog();
            }
        } else {
            log(Info) << "Configuration successful." <<endlog();
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
            
            TaskContext* peer = comps[ (*it)->getName() ].instance;
            // AutoStart
            if (comps[(*it)->getName()].autostart )
                if ( !peer || peer->start() == false) 
                    valid = false;
        }
        // Finally, report success/failure:
        if (!valid) {
            for ( CompList::iterator cit = comps.begin(); cit != comps.end(); ++cit) {
                ComponentData* it = &(cit->second);
                if ( it->instance == 0 ) {
                    log(Error) << "Failed to start component "<< cit->first << ": not found." << endlog();
                    continue;
                }
                if ( it->autostart && it->instance->getTaskState() != TaskCore::Running )
                    log(Error) << "Failed to start component "<< it->instance->getName() <<endlog();
            }
        } else {
            log(Info) << "Startup successful." <<endlog();
        }
        return valid;
    }

    bool DeploymentComponent::stopComponents()
    {
        Logger::In in("DeploymentComponent::stopComponents");
        bool valid = true;
        // 1. Stop all activities, give components chance to cleanup.
        for ( CompList::iterator cit = comps.begin(); cit != comps.end(); ++cit) {
            ComponentData* it = &(cit->second);
            if ( it->instance && !it->proxy )
                if ( it->instance->engine()->getActivity() == 0 || 
                     it->instance->engine()->getActivity()->isActive() == false ||
                     it->instance->stop() ) {
                    log(Info) << "Stopped "<< it->instance->getName() <<endlog();
                } else {
                    log(Error) << "Could not stop loaded Component "<< it->instance->getName() <<endlog();
                    valid = false;
                }
        }
        return valid;
    }

    bool DeploymentComponent::cleanupComponents()
    {
        Logger::In in("DeploymentComponent::cleanupComponents");
        bool valid = true;
        // 1. Cleanup all activities, give components chance to cleanup.
        for ( CompList::iterator cit = comps.begin(); cit != comps.end(); ++cit) {
            ComponentData* it = &(cit->second);
            if (it->instance && !it->proxy)
                if ( it->instance->getTaskState() <= TaskCore::Stopped ) {
                    it->instance->cleanup();
                    log(Info) << "Cleaned up "<< it->instance->getName() <<endlog();
                } else {
                    log(Error) << "Could not cleanup Component "<< it->instance->getName() << " (not Stopped)"<<endlog();
                    valid = false;
                }
        }
        return valid;
    }

    bool DeploymentComponent::unloadComponents()
    {
        // 2. Disconnect and destroy all components.
        bool valid = true;
        for ( CompList::iterator cit = comps.begin(); cit != comps.end(); ++cit) {
            ComponentData* it = &(cit->second);
          
            if ( it->loaded && it->instance ) {
                if ( it->instance->engine()->getActivity() == 0 ||
                     it->instance->engine()->getActivity()->isActive() == false ) {
                    std::string name = cit->first;
                    if (!it->proxy ) {
                        log(Debug) << "Disconnecting " <<name <<endlog();
                        it->instance->disconnect();
                        log(Debug) << "Terminating " <<name <<endlog();
                    } else
                        log(Debug) << "Removing proxy for " <<name <<endlog();
                    // delete the activity before the TC !
                    delete it->act;
                    it->act = 0;
                    delete it->instance;
                    it->instance = 0;
                    log(Info) << "Disconnected and destroyed "<< name <<endlog();
                } else {
                    log(Error) << "Could not unload Component "<< cit->first <<endlog();
                    valid = false;
                }
            }
        }
        if ( !comps.empty() ) {
            // cleanup from ComponentData map:
            CompList::iterator cit = comps.begin(); 
            do {
                ComponentData* it = &(cit->second);
                // if deleted and loaded by us:
                if (it->instance == 0 && it->loaded) {
                    log(Info) << "Completely removed "<< cit->first <<endlog();
                    comps.erase(cit);
                    cit = comps.begin();
                } else {
                    log(Info) << "Keeping info on "<< cit->first <<endlog();
                    ++cit;
                }
            } while ( cit != comps.end() );
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

        log(Debug) << "Importing " <<  path << endlog();

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
                        // Import only accepts libraries ending in .so
                        log(Debug) << "Scanning " << path +"/" +name <<endlog();
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
        // Accepted abbreviated syntax:
        // loadLibrary("liborocos-helloworld-gnulinux.so")
        // loadLibrary("liborocos-helloworld-gnulinux")
        // loadLibrary("liborocos-helloworld")
        // loadLibrary("orocos-helloworld")
        // With a path:
        // loadLibrary("/usr/lib/liborocos-helloworld-gnulinux.so")
        // loadLibrary("/usr/lib/liborocos-helloworld-gnulinux")
        // loadLibrary("/usr/lib/liborocos-helloworld")

        // Discover what 'name' is.
        bool name_is_path = false;
        bool name_is_so = false;
        if ( name.find("/") != std::string::npos )
            name_is_path = true;

        std::string so_name(name);
        if ( so_name.rfind(".so") == std::string::npos)
            so_name += ".so";
        else
            name_is_so = true;

        // Extract the libname such that we avoid double loading (crashes in case of two # versions).
        libname = name;
        if ( name_is_path ) 
            libname = libname.substr( so_name.rfind("/")+1 );
        if ( libname.find("lib") == 0 ) {
            libname = libname.substr(3);
        }
        // finally:
        libname = libname.substr(0, libname.find(".so") );

        for(vector<pair<string,void*> >::iterator it = LoadedLibs.begin(); it != LoadedLibs.end(); ++it){
            if ( it->first == libname ) {
                log(Info) <<"Library "<< libname <<" already loaded."<<endlog();
                handle = it->second;
                return true;
            }
        }

        std::vector<string> errors;
        // try form "liborocos-helloworld-gnulinux.so"
        handle = dlopen ( so_name.c_str(), RTLD_NOW | RTLD_GLOBAL );
        // if no path is given:
        if (!handle && !name_is_path ) {

            // with target provided:
            errors.push_back(string( dlerror() ));
            //cout << so_name.substr(0,3) <<endl;
            // try "orocos-helloworld-gnulinux.so"
            if ( so_name.substr(0,3) != "lib") {
                so_name = "lib" + so_name;
                handle = dlopen ( so_name.c_str(), RTLD_NOW | RTLD_GLOBAL);
                if (!handle)
                    errors.push_back(string( dlerror() ));
            }
            // try "liborocos-helloworld-gnulinux.so" in compPath
            if (!handle) {
                handle = dlopen ( string(compPath.get() + "/" + so_name).c_str(), RTLD_NOW | RTLD_GLOBAL);
            }

            if ( !name_is_so ) {
                // no so given, try to append target:
                so_name = name + "-" + target.get() + ".so";
                errors.push_back(string( dlerror() ));
                //cout << so_name.substr(0,3) <<endl;
                // try "orocos-helloworld"
                if ( so_name.substr(0,3) != "lib")
                    so_name = "lib" + so_name;
                handle = dlopen ( so_name.c_str(), RTLD_NOW | RTLD_GLOBAL);
                if (!handle)
                    errors.push_back(string( dlerror() ));
            }
            // try "liborocos-helloworld" in compPath
            if (!handle) {
                handle = dlopen ( string(compPath.get() + "/" + so_name).c_str(), RTLD_NOW | RTLD_GLOBAL);
            }

        }
        // if a path is given:
        if (!handle && name_is_path) {
            // just append target.so or .so"
            // with target provided:
            errors.push_back(string( dlerror() ));
            // try "/path/liborocos-helloworld-gnulinux.so" in compPath
            if (!handle) {
                handle = dlopen ( string(compPath.get() + "/" + so_name).c_str(), RTLD_NOW | RTLD_GLOBAL);
            }

            if ( !name_is_so ) {
                // no so given, try to append target:
                so_name = name + "-" + target.get() + ".so";
                errors.push_back(string( dlerror() ));
                //cout << so_name.substr(0,3) <<endl;
                // try "/path/liborocos-helloworld"
                handle = dlopen ( so_name.c_str(), RTLD_NOW | RTLD_GLOBAL);
                if (!handle)
                    errors.push_back(string( dlerror() ));
            }

            // try "/path/liborocos-helloworld" in compPath
            if (!handle) {
                handle = dlopen ( string(compPath.get() + "/" + so_name).c_str(), RTLD_NOW | RTLD_GLOBAL);
            }
        }

        if (!handle) {
            errors.push_back(string( dlerror() ));
            log(Error) << "Could not load library '"<< name <<"':"<<endlog();
            for(vector<string>::iterator i=errors.begin(); i!= errors.end(); ++i)
                log(Error) << *i << endlog();
            return false;
        }

        LoadedLibs.push_back(make_pair(libname,handle));
        log(Debug) <<"Storing "<< libname <<endlog();
        dlerror();    /* Clear any existing error */
        
        // Lookup Component factories:
        FactoryMap* (*getfactory)(void) = 0;
        FactoryMap* fmap = 0;
        char* error = 0;
        bool is_component = false;
        getfactory = (FactoryMap*(*)(void))( dlsym(handle, "getComponentFactoryMap") );
        if ((error = dlerror()) == NULL) {
            // symbol found, register factories...
            fmap = (*getfactory)();
            ComponentFactories::Instance().insert( fmap->begin(), fmap->end() );
            log(Info) << "Loaded multi component library '"<<so_name <<"'"<<endlog();
            log(Debug) << "Components:";
            for (FactoryMap::iterator it = fmap->begin(); it != fmap->end(); ++it)
                log(Debug) <<" "<<it->first;
            log(Debug) << endlog();
            is_component = true;
        } else {
            log(Debug) << error << endlog();
        }

        // Lookup createComponent:
        dlerror();    /* Clear any existing error */

        TaskContext* (*factory)(std::string) = 0;
        std::string(*tname)(void) = 0;
        factory = (TaskContext*(*)(std::string))(dlsym(handle, "createComponent") );
        if ((error = dlerror()) == NULL) {
            // store factory.
            if ( ComponentFactories::Instance().count(libname) == 1 ) {
                log(Warning) << "Library name "<<libname<<" already used: overriding."<<endlog();
            }
            ComponentFactories::Instance()[libname] = factory;
            tname = (std::string(*)(void))(dlsym(handle, "getComponentType") );
            if ((error = dlerror()) == NULL) {
                std::string cname = (*tname)();
                if ( ComponentFactories::Instance().count(cname) == 1 ) {
                    log(Warning) << "Component type name "<<cname<<" already used: overriding."<<endlog();
                }
                ComponentFactories::Instance()[ cname ] = factory;
                log(Info) << "Loaded component type '"<< cname <<"'"<<endlog();
            } else {
                log(Info) << "Loaded single component library '"<< libname <<"'"<<endlog();
            }
            is_component = true;
        } else {
            log(Debug) << error << endlog();
        }

        // Lookup plugins:
        dlerror();    /* Clear any existing error */

        bool (*loadPlugin)(TaskContext*) = 0;
        std::string(*pluginName)(void) = 0;
        loadPlugin = (bool(*)(TaskContext*))(dlsym(handle, "loadRTTPlugin") );
        if ((error = dlerror()) == NULL) {
            string plugname;
            pluginName = (std::string(*)(void))(dlsym(handle, "getRTTPluginName") );
            if ((error = dlerror()) == NULL) {
                plugname = (*pluginName)();
            } else {
                plugname  = libname;
            }

            // ok; try to load it.
            bool success = false;
            try {
                success = (*loadPlugin)(this);
            } catch(...) {
                log(Error) << "Unexpected exception in loadRTTPlugin !"<<endlog();
            }

            if ( success )
                log(Info) << "Loaded RTT Plugin '"<< plugname <<"'"<<endlog();
            else {
                log(Error) << "Failed to load RTT Plutin '" <<plugname<<"': plugin refused to load into this TaskContext." <<endlog();
                return false;
            }
            return true;
        } else {
            log(Debug) << error << endlog();
        }


        // plain library
        if (is_component == false)
            log(Info) << "Loaded shared library '"<< so_name <<"'"<<endlog();
        return true;
    }

    // or type is a shared library or it is a class type.
    bool DeploymentComponent::loadComponent(const std::string& name, const std::string& type)
    {
        Logger::In in("DeploymentComponent::loadComponent");

        if ( type == "PropertyBag" )
            return false; // It should be present as peer.

        if ( this->getPeer(name) || ( comps.find(name) != comps.end() && comps[name].instance != 0) ) {
            log(Error) <<"Failed to load component with name "<<name<<": already present as peer or loaded."<<endlog();
            return false;
        }

        TaskContext* (*factory)(std::string name) = 0;
        char *error;

        // First: try loading from imported libraries. (see: import).
        if ( ComponentFactories::Instance().count(type) == 1 ) {
            factory = ComponentFactories::Instance()[ type ];
            if (factory == 0 ) {
                log(Error) <<"Found empty factory for Component type "<<type<<endlog();
                return false;
            }
        }
                
        if ( factory ) {
            log(Debug) <<"Found factory for Component type "<<type<<endlog();
        } else {
            // if a type was given, bail out immediately.
            if ( type.find("::") != string::npos) {
                log(Error) << "Unable to locate Orocos plugin '"<<type<<"': unknown component type." <<endlog();
                return false;
            }
            // Second: try dynamic loading:
            if ( loadLibrary(type) == false )
                return false;

            dlerror();    /* Clear any existing error */
            factory = (TaskContext*(*)(std::string))(dlsym(handle, "createComponent"));
            if ((error = dlerror()) != NULL) {
                log(Error) << "Found plugin '"<< type <<"', but it can not create a single Orocos Component:";
                log(Error) << error << endlog();
                // leave it loaded.
                return false;
            }
            log(Info) << "Found Orocos plugin '"<< type <<"'"<<endlog();
        }
        
        try {
            comps[name].instance = (*factory)(name);
        } catch(...) {
            log(Error) <<"The constructor of component type "<<type<<" threw an exception!"<<endlog();
        }

        if ( comps[name].instance == 0 ) {
            log(Error) <<"Failed to load component with name "<<name<<": refused to be created."<<endlog();
            return false;
        }

        if (!this->componentLoaded( comps[name].instance ) ) {
            log(Error) << "This deployer type refused to connect to "<< comps[name].instance->getName() << ": aborting !" << endlog(Error);
            delete comps[name].instance;
            return false;
        }

        // unlikely that this fails (checked at entry)!
        this->addPeer( comps[name].instance );
        log(Info) << "Adding "<< comps[name].instance->getName() << " as new peer:  OK."<< endlog(Info);
        comps[name].loaded = true;
        return true;
    }


    bool DeploymentComponent::unloadComponent(const std::string& name)
    {
        std::string regname = name;
        TaskContext* peer = this->getPeer(name);
        if (peer) {
            for(CompList::iterator it = comps.begin(); it != comps.end(); ++it)
                if (it->second.instance == peer) {
                    regname = it->first;
                    break;
                }
        } else {
            // no such peer: try looking for the map name
            if ( comps.count( name ) == 0 || comps[name].loaded == false ) {
                log(Error) << "Can't unload component '"<<name<<"': not loaded by "<<this->getName()<<endlog();
                return false;
                }
            // regname and name are equal.
            peer = comps[name].instance;
        }
            
        assert(peer);
        if ( peer->isRunning() ) {
            log(Error) << "Can't unload component "<<name<<" since it is still running."<<endlog();
            return false;
        }
        try {
            peer->disconnect(); // if it is no longer a peer of this, that's ok.
        } catch(...) {
            log(Warning) << "Disconnecting caused exception." <<endlog();
        }
        delete peer;
        comps.erase(regname);
        log(Info) << "Successfully unloaded component "<<name<<"."<<endlog();
        return true;
    }

    void DeploymentComponent::displayComponentTypes() const
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
        TaskContext* peer = 0;
        if ( comp_name == this->getName() )
            peer = this;
        else
            if ( comps.count(comp_name) ) 
                peer = comps[comp_name].instance;
            else
                peer = this->getPeer(comp_name); // last resort.
        if (!peer) {
            log(Error) << "Can't create Activity: component "<<comp_name<<" not found."<<endlog();
            return false;
        }
        // this is required for lateron attaching the engine()
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

    bool DeploymentComponent::configure(const std::string& name)
    {
        return configureFromFile( name,  name + ".cpf" );
    }

    bool DeploymentComponent::configureFromFile(const std::string& name, const std::string& filename)
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

    FactoryMap& DeploymentComponent::getFactories()
    {
        return ComponentFactories::Instance();
    }
}
