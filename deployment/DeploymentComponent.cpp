/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:34:40 CEST 2008  DeploymentComponent.cpp

                        DeploymentComponent.cpp -  description
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



#include <rtt/RTT.hpp>
#include "DeploymentComponent.hpp"
#include <rtt/Activities.hpp>
#include <rtt/marsh/PropertyMarshaller.hpp>
#include <rtt/marsh/PropertyDemarshaller.hpp>

#include <cstdio>
#include <dlfcn.h>
#include "ocl/ComponentLoader.hpp"
#include <rtt/PropertyLoader.hpp>

#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <set>

// chose the file extension applicable to the O/S
#ifdef  __APPLE__
static const std::string SO_EXT(".dylib");
#else
static const std::string SO_EXT(".so");
#endif

using namespace Orocos;

namespace OCL
{
    using namespace std;
    using namespace RTT;

    std::vector< DeploymentComponent::LoadedLib > DeploymentComponent::loadedLibs;

    /**
     * I'm using a set to speed up lookups.
     */
    static std::set<string> valid_names;

#define ORO_str(s) ORO__str(s)
#define ORO__str(s) #s

    DeploymentComponent::DeploymentComponent(std::string name, std::string siteFile)
        : RTT::TaskContext(name, Stopped),
          compPath("ComponentPath",
                   "Location to look for components in addition to the local directory and system paths.",
                   "/usr/lib"),
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
        this->methods()->addMethod( RTT::method("kickOutAll", &DeploymentComponent::kickOutAll, this),
                                    "Calls stopComponents, cleanupComponents and unloadComponents in a row.");

        this->methods()->addMethod( RTT::method("kickOutComponent", &DeploymentComponent::kickOutComponent, this),
                                    "Calls stopComponents, cleanupComponent and unloadComponent in a row.",
                                    "comp_name", "component name");
        this->methods()->addMethod( RTT::method("kickOut", &DeploymentComponent::kickOut, this),
                                    "Calls stopComponents, cleanupComponents and unloadComponents in a row.",
                                    "File", "The file which contains the name of the components to kickOut (for example, the same used in kickStart).");

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
                                    "Attach a 'stand alone' slave activity to a Component.",
                                    "CompName", "The name of the Component.",
                                    "Period", "The period of the activity (set to zero for non periodic)."
                                    );
        this->methods()->addMethod( RTT::method("setMasterSlaveActivity", &DeploymentComponent::setMasterSlaveActivity, this),
                                    "Attach a slave activity with a master to a Component. The slave becomes a peer of the master as well.",
                                    "Master", "The name of the Component which is master of the Slave.",
                                    "Slave", "The name of the Component which gets the SlaveActivity."
                                    );

        valid_names.insert("AutoUnload");
        valid_names.insert("UseNamingService");
        valid_names.insert("Server");
        valid_names.insert("AutoConf");
        valid_names.insert("AutoStart");
        valid_names.insert("AutoConnect");
        valid_names.insert("AutoSave");
        valid_names.insert("PropertyFile");
        valid_names.insert("UpdateProperties");
        valid_names.insert("LoadProperties");
        valid_names.insert("ProgramScript");
        valid_names.insert("StateMachineScript");
        valid_names.insert("Ports");
        valid_names.insert("Peers");
        valid_names.insert("Activity");
        valid_names.insert("Master");
        valid_names.insert("Properties");

        // Check for 'Deployer-site.cpf' XML file.
        if (siteFile.empty())
            siteFile = this->getName()  "-site.cpf";
        std::ifstream hassite(siteFile);
        if ( !hassite ) {
            // if not, just configure
            this->configure();
            return;
        }

        // OK: kick-start it. Need to set AutoConf to configure self.
        this->kickStart( siteFile );

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
      // Should we unload all loaded components here ?
      if ( autoUnload.get() ) {
          kickOutAll();
      }
    }

    bool DeploymentComponent::connectPeers(const std::string& one, const std::string& other)
    {
        Logger::In in("DeploymentComponent::connectPeers");
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
        Logger::In in("DeploymentComponent::addPeer");
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
	Logger::In in("DeploymentComponent::connectPorts");
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
	Logger::In in("DeploymentComponent::connectPorts");
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

    bool DeploymentComponent::kickOutAll()
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
                            log(Error)<< "Property '"<< *it <<"' should be a struct, Include or Import statement." << endlog();
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
                            if ( (*optit)->getName() == "AutoSave" ) {
                                Property<bool> ps = comp.rvalue().getProperty<bool>("AutoSave");
                                if (!ps.ready()) {
                                    log(Error) << "AutoSave must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].autosave = ps.get();
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
                                Property<string> ps = comp.rvalue().getProperty<string>("PropertyFile");
                                if (!ps.ready()) {
                                    log(Error) << "PropertyFile must be of type <string>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].configfile = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "UpdateProperties" ) {
                                Property<string> ps = comp.rvalue().getProperty<string>("UpdateProperties");
                                if (!ps.ready()) {
                                    log(Error) << "UpdateProperties must be of type <string>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].configfile = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "LoadProperties" ) {
                                Property<string> ps = comp.rvalue().getProperty<string>("LoadProperties");
                                if (!ps.ready()) {
                                    log(Error) << "LoadProperties must be of type <string>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].configfile = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "Properties" ) {
                                PropertyBase* ps = comp.rvalue().getProperty<PropertyBag>("Properties");
                                if (!ps) {
                                    log(Error) << "Properties must be a <struct>" << endlog();
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

                        // Check if we know or are this component.
                        TaskContext* c = 0;
                        if ( (*it)->getName() == this->getName() )
                            c = this;
                        else
                            c = this->getPeer( (*it)->getName() );
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
                                if (valid){
                                    string port_name = ports->get().getProperty<string>(*pit)->get();
                                    bool to_add = true;
                                    // go through the vector to avoid duplicate items.
                                    // NOTE the sizes conmap[port_name].ports.size() and conmap[port_name].owners.size() are supposed to be equal
                                    for(unsigned int a=0; a < conmap[port_name].ports.size(); a++)
                                        {
                                            if(  conmap[port_name].ports.at(a) == p && conmap[port_name].owners.at(a) == c)
                                                {
                                                    to_add = false;
                                                    continue;
                                                }
                                        }

                                    if(to_add)
                                        {
                                            log(Debug)<<"storing Port: "<<c->getName()<<"."<<p->getName();
                                            log(Debug)<<" in " << port_name <<endlog();
                                            conmap[port_name].ports.push_back( p );
                                            conmap[port_name].owners.push_back( c );
                                        }
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
                                            string master;
                                            if ( nm.rvalue().getProperty<string>("Master") ) {
                                                master = nm.rvalue().getProperty<string>("Master")->get();
                                                if (valid) {
                                                    this->setActivity(comp.getName(), nm.rvalue().getType(), period, 0, 0, master );
                                                }
                                            } else {
                                                // No master given.
                                                if ( nm.rvalue().getProperty<double>("Period") )
                                                    period = nm.rvalue().getProperty<double>("Period")->get();
                                                if (valid) {
                                                    this->setActivity(comp.getName(), nm.rvalue().getType(), period, 0, 0 );
                                                }
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
                        {
                            this->addPeer( comps[comp.getName()].instance->getName(), nm.value() );
                            log(Debug) << this->getName() << " connects to " <<
                                comps[comp.getName()].instance->getName()<< nm.value()  << endlog();
                        }
                    else {
                        log(Error) << "Wrong property type in Peers struct. Expected property of type 'string',"
                                   << " got type "<< (*it)->getType() <<endlog();
                        valid = false;
                    }
                }
        }

        // Create data port connections:
        for(ConMap::iterator it = conmap.begin(); it != conmap.end(); ++it) {
            ConnectionData *connection =  &(it->second);
            std::string connection_name = it->first;

            if ( connection->ports.size() == 1 ){
                log(Warning) << "Can not form connection "<<connection_name<<" with only one Port from "<< connection->owners[0]->getName()<< endlog();
                continue;
            }
            // first find a write and a read port.
            // This is quite complex since a 'ReadWritePort' can act as both.
            PortInterface* writer = 0, *reader = 0;
            ConnectionData::Ports::iterator p = connection->ports.begin();

            // If one of the ports is connected, use that one as writer to connect to.
            while (p !=connection->ports.end() && (writer == 0 ) ) {
                if ( (*p)->connected() )
                    writer = *p;
                ++p;
            }

            // now proceed filling in writer and reader pointers until one reader and one writer is found.
            p = connection->ports.begin();
            while (p !=connection->ports.end() && (writer == 0 || reader == 0) ) {
                if ( (*p)->getPortType() == PortInterface::WritePort )
                    {
                        if (!reader && writer && writer->getPortType() == PortInterface::ReadWritePort && !writer->connected() ) {
                            reader = writer;
                            writer = 0;
                        }
                        if (!writer)
                            writer = (*p);
                    }
                else{
                    if ( (*p)->getPortType() == PortInterface::ReadPort )
                        {
                            if (!writer && reader && reader->getPortType() == PortInterface::ReadWritePort ) {
                                writer = reader;
                                reader = 0;
                            }
                            if (!reader)
                                reader = (*p);
                        }
                    else{
                        if ( (*p)->getPortType() == PortInterface::ReadWritePort )
                            {
                                if (writer == 0)
                                    {
                                        writer = (*p);
                                    }
                                else {
                                    reader = (*p);
                                }
                            }
                    }
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
            // Inform user which component initiates the connection:
            p = connection->ports.begin();
            while ( *p != writer ) ++p;
            std::string owner = it->second.owners[p - it->second.ports.begin()]->getName();
            log(Info) << "Creating Connection "<<it->first<<" starting from "<< owner <<"."<<writer->getName()<<" :" <<endlog();
            // connect all ports to connection
            p = connection->ports.begin();

            while (p != connection->ports.end() ) {
                // connect all readers to the first found writer.
                if ( *p != writer )
                    {
                        owner = connection->owners[p - connection->ports.begin()]->getName();
                        // only try to connect p if it is not in the same connection of writer.
                        if ( ( writer->connected() && (*p)->connection() == writer->connection() ) ) {
                            ++p;
                            continue;
                        }
                        // OK. p is definately no part of writer's connection. Try to connect and flag errors if it fails.
                        if ( (*p)->connectTo( writer ) == false) {
                            log(Error) << "Could not connect Port "<< owner<<"."<< (*p)->getName() << " to connection " << connection_name <<endlog();
                            if ((*p)->connected())
                                log(Error) << "Port "<< owner<<"."<< (*p)->getName() << " already connected to other connection !"<<endlog();
                            else
                                log(Error) << "Port "<< owner<<"."<< (*p)->getName() << " has wrong type !"<<endlog();
                            valid = false;
                        } else {
                            log(Info) << "Connected Port "<< owner<<"."<< (*p)->getName() <<" to connection " << connection_name <<endlog();
                        }
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

            // do not configure when not stopped.
            if ( peer->getTaskState() > Stopped) {
                log(Warning) << "Component "<< peer->getName()<< " doesn't need to be configured (already Running)." <<endlog();
                continue;
            }

            // Check for default properties to set.
            for (PropertyBag::const_iterator pf = comp.rvalue().begin(); pf!= comp.rvalue().end(); ++pf) {
                // set PropFile name if present
                if ( (*pf)->getName() == "Properties"){
                    Property<PropertyBag> props = *pf; // convert to type.
                    bool ret = updateProperties( *peer->properties(), props);
                    if (!ret) {
                        log(Error) << "Failed to configure properties from main configuration file for component "<< comp.getName() <<endlog();
                        valid = false;
                    } else {
                        log(Info) << "Configured Properties of "<< comp.getName() <<" from main configuration file." <<endlog();
                    }
                }
            }
            // Load/update from property files.
            for (PropertyBag::const_iterator pf = comp.rvalue().begin(); pf!= comp.rvalue().end(); ++pf) {
                // set PropFile name if present
                if ( (*pf)->getName() == "PropertyFile" || (*pf)->getName() == "UpdateProperties" || (*pf)->getName() == "LoadProperties"){
                    dummy = *pf; // convert to type.
                    string filename = dummy.get();
                    PropertyLoader pl;
                    bool strict = (*pf)->getName() == "PropertyFile" ? true : false;
                    bool load = (*pf)->getName() == "LoadProperties" ? true : false;
                    bool ret;
                    if (!load)
                        ret = pl.configure( filename, peer, strict );
                    else
                        ret = pl.load(filename, peer);
                    if (!ret) {
                        log(Error) << "Failed to configure properties for component "<< comp.getName() <<endlog();
                        valid = false;
                    } else {
                        log(Info) << "Configured Properties of "<< comp.getName() << " from "<<filename<<endlog();
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
                {
                    if( !peer->isActive() )
                        {
                            if ( peer->configure() == false)
                                valid = false;
                        }
                    else
                        log(Warning) << "Apparently component "<< peer->getName()<< " don't need to be configured." <<endlog();
                }
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
            if ( it->instance && !it->proxy ) {
                if ( it->instance->engine()->getActivity() == 0 ||
                     it->instance->engine()->getActivity()->isActive() == false ||
                     it->instance->stop() ) {
                    log(Info) << "Stopped "<< it->instance->getName() <<endlog();
                } else {
                    log(Error) << "Could not stop loaded Component "<< it->instance->getName() <<endlog();
                    valid = false;
                }
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
            if (it->instance && !it->proxy) {
                if ( it->instance->getTaskState() <= TaskCore::Stopped ) {
                    if ( it->autosave && !it->configfile.empty()) {
                        string file = it->configfile; // get file name
                        PropertyLoader pl;
                        bool ret = pl.save( file, it->instance, true ); // save all !
                        if (!ret) {
                            log(Error) << "Failed to save properties for component "<< it->instance->getName() <<endlog();
                            valid = false;
                        } else {
                            log(Info) << "Saved Properties of "<< it->instance->getName() << " to "<<file<<endlog();
                        }
                    }
                    it->instance->cleanup();
                    log(Info) << "Cleaned up "<< it->instance->getName() <<endlog();
                } else {
                    log(Error) << "Could not cleanup Component "<< it->instance->getName() << " (not Stopped)"<<endlog();
                    valid = false;
                }
            }
        }
        return valid;
    }

    bool DeploymentComponent::unloadComponents()
    {
        // 2. Disconnect and destroy all components.
        bool valid = true;
        while ( comps.size() > 0)
            {
                CompList::iterator cit = comps.begin();
                valid &= this->unloadComponentImpl(cit);
            }
        return valid;
    }

    void DeploymentComponent::clearConfiguration()
    {
        log(Info) << "Clearing configuration options."<< endlog();
        conmap.clear();
        deletePropertyBag( root );
    }

#ifdef __APPLE__
    int filter_so(struct dirent * d)
#else // Only tested on gnulinux so far
    int filter_so(const struct dirent * d)
#endif
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
                if (name != "." && name != ".." ) {
                    if (namelist[i]->d_type == DT_DIR) { //ignoring symlinks and subdirs here.
                        import( path +"/" +name );
                    } else {
                        // Import only accepts libraries ending in .so
                        log(Debug) << "Scanning " << path +"/" +name <<endlog();
                        if ( name.rfind(SO_EXT) == std::string::npos ||
                             name.substr(name.rfind(SO_EXT)) != SO_EXT ) {
                            log(Debug) << "Dropping " << name <<endlog();
                        }
                        else {
                            log(Debug) << "Accepting " << name <<endlog();
                            loadLibrary( path + "/" + name );
                        }
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
        if ( so_name.rfind(SO_EXT) == std::string::npos)
            so_name += SO_EXT;
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
        libname = libname.substr(0, libname.find(SO_EXT) );

        // check if the library is already loaded
        // NOTE if this library has been loaded, you can unload and reload it to apply changes (may be you have updated the dynamic library)
        // anyway it is safe to do this only if thereisn't any istance whom type was loaded from this library

        std::vector<LoadedLib>::iterator lib = loadedLibs.begin();
        while (lib != loadedLibs.end()) {
            // there is already a library with the same name
            if ( lib->name == libname) {
                log(Warning) <<"Library "<< libname <<".so already loaded. " ;

                bool can_unload = true;
                CompList::iterator cit;
                for( std::vector<std::string>::iterator ctype = lib->components_type.begin();  ctype != lib->components_type.end() && can_unload; ++ctype) {
                    for ( cit = comps.begin(); cit != comps.end(); ++cit) {
                        if( (*ctype) == cit->second.type ) {
                            // the type of an allocated component was loaded from this library. it might be unsafe to reload the library
                            log(Warning) << "Can NOT reload because of the instance " << cit->second.type  <<"::"<<cit->second.instance->getName()  <<endlog();
                            can_unload = false;
                        }
                    }
                }
                if( can_unload ) {
                    log(Warning) << "Try to RELOAD"<<endlog();
                    dlclose(lib->handle);
                    // remove the library info from the vector
                    std::vector<LoadedLib>::iterator lib_un = lib;
                    loadedLibs.erase(lib_un);
                    lib = loadedLibs.end();
                }
                else   return true;
            }
            else lib++;
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
                so_name = name + "-" + target.get() + SO_EXT;
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
                so_name = name + "-" + target.get() + SO_EXT;
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

        //------------- if you get here, the library has been loaded -------------
        LoadedLib loading_lib(libname,handle);
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
                loading_lib.components_type.push_back( cname );

            } else {
                log(Info) << "Loaded single component library '"<< libname <<"'"<<endlog();
                loading_lib.components_type.push_back( libname );
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
                log(Error) << "Failed to load RTT Plugin '" <<plugname<<"': plugin refused to load into this TaskContext." <<endlog();
                return false;
            }
            return true;
        } else {
            log(Debug) << error << endlog();
        }

        loadedLibs.push_back( loading_lib );
        log(Info) <<"Storing "<< loading_lib.name  <<endlog();
        dlerror();    /* Clear any existing error */


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
        comps[name].type = type;
        return true;
    }

    /**
     * This method removes all references to the component hold in \a cit,
     * on the condition that it is not running.
     */
    bool DeploymentComponent::unloadComponentImpl( CompList::iterator cit )
    {
        bool valid = true;
        ComponentData* it = &(cit->second);
        std::string  name = cit->first;

        if ( it->loaded && it->instance ) {
            if ( it->instance->engine()->getActivity() == 0 ||
                 it->instance->engine()->getActivity()->isActive() == false ) {
                if (!it->proxy ) {
                    log(Debug) << "Disconnecting " <<name <<endlog();
                    it->instance->disconnect();
                    log(Debug) << "Terminating " <<name <<endlog();
                } else
                    log(Debug) << "Removing proxy for " <<name <<endlog();

                // Lookup and erase port+owner from conmap.
                for( ConMap::iterator cmit = conmap.begin(); cmit != conmap.end(); ++cmit) {
                    size_t n = 0;
                    while ( n != cmit->second.owners.size() ) {
                        if (cmit->second.owners[n] == it->instance ) {
                            cmit->second.owners.erase( cmit->second.owners.begin() + n );
                            cmit->second.ports.erase( cmit->second.ports.begin() + n );
                            n = 0;
                        } else
                            ++n;
                    }
                }
                // Lookup in the property configuration and remove:
                Property<PropertyBag>* pcomp = root.getProperty<PropertyBag>(name);
                if (pcomp) {
                    root.remove(pcomp);
                    deletePropertyBag( pcomp->value() );
                    delete pcomp;
                }

                // Finally, delete the activity before the TC !
                delete it->act;
                it->act = 0;
                delete it->instance;
                it->instance = 0;
                log(Info) << "Disconnected and destroyed "<< name <<endlog();
            } else {
                log(Error) << "Could not unload Component "<< name <<": still running." <<endlog();
                valid=false;
            }
        }
        if (valid) {
            // NOTE there is no reason to keep the ComponentData in the vector.
            // actually it may cause errors if we try to re-load the Component later.
            comps.erase(cit);
        }
        return valid;
    }

    bool DeploymentComponent::unloadComponent(const std::string& name)
    {
        CompList::iterator it;
            // no such peer: try looking for the map name
            if ( comps.count( name ) == 0 || comps[name].loaded == false ) {
                log(Error) << "Can't unload component '"<<name<<"': not loaded by "<<this->getName()<<endlog();
                return false;
                }

        // Ok. Go on with loaded component.
        it = comps.find(name);

        if ( this->unloadComponentImpl( it ) == false )
            return false;

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

    bool DeploymentComponent::setMasterSlaveActivity(const std::string& master,
                                                   const std::string& slave)
    {
        if ( this->setActivity(slave, "SlaveActivity", 0, 0, ORO_SCHED_OTHER, master ) ) {
            assert( comps[slave].instance );
            assert( comps[slave].act );
            comps[slave].act->run(comps[slave].instance->engine());
            return true;
        }
        return false;
    }


    bool DeploymentComponent::setActivity(const std::string& comp_name,
                                          const std::string& act_type,
                                          double period, int priority,
                                          int scheduler, const std::string& master_name)
    {
        TaskContext* peer = 0;
        ActivityInterface* master_act = 0;
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
        if ( !master_name.empty() ) {
            if ( master_name == this->getName() )
	        master_act = this->engine()->getActivity();
            else
                if ( comps.count(master_name) )
		    master_act = comps[master_name].act;
                else
		    master_act = this->getPeer(master_name) ? getPeer(master_name)->engine()->getActivity() : 0; // last resort.

	    if ( !this->getPeer(master_name) ) {
                log(Error) << "Can't create SlaveActivity: Master component "<<master_name<<" not known as peer."<<endlog();
                return false;
            }

            if (!master_act) {
                log(Error) << "Can't create SlaveActivity: Master component "<<master_name<<" has no activity set."<<endlog();
                return false;
            }
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
                if ( act_type == "SlaveActivity" ) {
                    if ( master_act == 0 )
                        newact = new SlaveActivity(period);
                    else {
		        newact = new SlaveActivity(master_act);
                        this->getPeer(master_name)->addPeer( peer );
                    }
                }

        if (newact == 0) {
            log(Error) << "Can't create "<< act_type << " for component "<<comp_name<<": incorrect arguments."<<endlog();
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

    void DeploymentComponent::kickOut(const std::string& config_file)
    {
        Logger::In in("DeploymentComponent::kickOut");
        PropertyBag from_file;
        Property<std::string>  import_file;
        std::vector<std::string> deleted_components_type;

        // demarshalling failures:
        bool failure = false;

        PropertyDemarshaller demarshaller(config_file);
        try {
            if ( demarshaller.deserialize( from_file ) ){
                for (PropertyBag::iterator it= from_file.begin(); it!=from_file.end();it++) {
                    if ( (*it)->getName() == "Import" ) continue;
                    if ( (*it)->getName() == "Include" ) continue;

                    kickOutComponent(  (*it)->getName() );
                }
                deletePropertyBag( from_file );
            }
            else {
                log(Error)<< "Some error occured while parsing "<< config_file <<endlog();
                failure = true;
            }
        } catch (...)
            {
                log(Error)<< "Uncaught exception in loadcomponents() !"<< endlog();
                failure = true;
            }
    }

    bool DeploymentComponent::cleanupComponent(RTT::TaskContext *instance)
    {
        Logger::In in("DeploymentComponent::cleanupComponent");
        bool valid = true;
        // 1. Cleanup a single activities, give components chance to cleanup.
        if (instance) {
            if ( instance->getTaskState() <= TaskCore::Stopped ) {
                instance->cleanup();
                log(Info) << "Cleaned up "<< instance->getName() <<endlog();
            } else {
                log(Error) << "Could not cleanup Component "<< instance->getName() << " (not Stopped)"<<endlog();
                valid = false;
            }
        }
        return valid;
    }

    bool DeploymentComponent::stopComponent(RTT::TaskContext *instance)
    {
        Logger::In in("DeploymentComponent::stopComponent");
        bool valid = true;

        if ( instance ) {
            if ( instance->engine()->getActivity() == 0 ||
                 instance->engine()->getActivity()->isActive() == false ||
                 instance->stop() ) {
                log(Info) << "Stopped "<< instance->getName() <<endlog();
            }
            else {
                log(Error) << "Could not stop loaded Component "<< instance->getName() <<endlog();
                valid = false;
            }
        }
        return valid;
    }

    bool DeploymentComponent::kickOutComponent(const std::string& comp_name)
    {
        Logger::In in("DeploymentComponent::kickOutComponent");
        PropertyBase *it = root.find( comp_name );
        if(!it) {
            log(Error) << "Peer "<< comp_name << " not found in PropertyBag root"<< endlog();
            return false;
        }

        TaskContext* peer = comps[ comp_name ].instance;

        if ( !peer ) {
            log(Error) << "Peer not found: "<< comp_name <<endlog();
            return false;
        }
        stopComponent( peer );
        cleanupComponent (peer );
        unloadComponent( comp_name);
        root.removeProperty( root.find( comp_name ) );

        return true;
    }
}
