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
#include <rtt/Activity.hpp>
#include <rtt/marsh/PropertyMarshaller.hpp>
#include <rtt/marsh/PropertyDemarshaller.hpp>

#include <cstdio>
#include "ocl/ComponentLoader.hpp"
#include "ComponentLoader.hpp"
#include <rtt/PropertyLoader.hpp>
#include <rtt/os/PluginLoader.hpp>

#include <iostream>
#include <fstream>
#include <set>

using namespace Orocos;

namespace OCL
{
    using namespace std;
    using namespace RTT;
    using namespace RTT::OS;

    /**
     * I'm using a set to speed up lookups.
     */
    static std::set<string> valid_names;

#define ORO_str(s) ORO__str(s)
#define ORO__str(s) #s

    DeploymentComponent::DeploymentComponent(std::string name, std::string siteFile)
        : RTT::TaskContext(name, Stopped),
	  compPath("RTT_COMPONENT_PATH", "Locations to look for components. Use a colon or semi-colon separated list of paths. Defaults to the environment variable with the same name."),
          autoUnload("AutoUnload",
                     "Stop, cleanup and unload all components loaded by the DeploymentComponent when it is destroyed.",
                     true),
          validConfig("Valid", false),
          sched_RT("ORO_SCHED_RT", ORO_SCHED_RT ),
          sched_OTHER("ORO_SCHED_OTHER", ORO_SCHED_OTHER ),
          lowest_Priority("LowestPriority", RTT::OS::LowestPriority ),
          highest_Priority("HighestPriority", RTT::OS::HighestPriority ),
          target("Target",
                 ORO_str(OROCOS_TARGET) )
    {
        this->properties()->addProperty( &compPath );
        this->properties()->addProperty( &autoUnload );

        this->attributes()->addAttribute( &target );
        this->attributes()->addAttribute( &validConfig );
        this->attributes()->addAttribute( &sched_RT );
        this->attributes()->addAttribute( &sched_OTHER );
        this->attributes()->addAttribute( &lowest_Priority );
        this->attributes()->addAttribute( &highest_Priority );


        this->methods()->addMethod( RTT::method("loadLibrary", &DeploymentComponent::loadLibrary, this),
                                    "Load a new library into memory. This may be an absolute filename or of the form subdirs/foo where libfoo.so must be loaded in the RTT_COMPONENT_PATH.",
                                    "Name", "The (relative) filename of the to be loaded library.");
        this->methods()->addMethod( RTT::method("import", &DeploymentComponent::import, this),
                                    "Load all libraries in Path into memory. The Path is relative to the RTT_COMPONENT_PATH.",
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

        this->methods()->addMethod( RTT::method("setActivity", &DeploymentComponent::setActivity, this),
                                    "Attach an activity to a Component.",
                                    "CompName", "The name of the Component.",
                                    "Period", "The period of the activity (set to 0.0 for non periodic).",
                                    "Priority", "The priority of the activity.",
                                    "SchedType", "The scheduler type of the activity."
                                    );
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
        this->methods()->addMethod( RTT::method("setSequentialActivity", &DeploymentComponent::setSequentialActivity, this),
                                    "Attach a 'stand alone' sequential activity to a Component.",
                                    "CompName", "The name of the Component."
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
            siteFile = this->getName() + "-site.cpf";
        std::ifstream hassite(siteFile.c_str());
        if ( !hassite ) {
            // if not, just configure
            this->configure();
            log(Debug) << "Not using site file." << endlog();
            return;
        }

        // OK: kick-start it. Need to set AutoConf to configure self.
        log(Info) << "Using site file '" << siteFile << "'." << endlog();
        this->kickStart( siteFile );

    }

    bool DeploymentComponent::configureHook()
    {
      char* paths = getenv("RTT_COMPONENT_PATH");
      if (compPath.value().empty() )
        {
	  if (paths) {
	    compPath = string(paths);
	  } else {
	    log(Info) <<"No RTT_COMPONENT_PATH set. Using default." <<endlog();
	    compPath = default_comp_path ;
	  }
        }
      log(Info) <<"RTT_COMPONENT_PATH was set to " << compPath << endlog();
      log(Info) <<"Re-scanning for plugins and components..."<<endlog();
      PluginLoader::Instance()->setPluginPath(compPath);
      PluginLoader::Instance()->loadTypekits("");
      PluginLoader::Instance()->loadPlugins("");
      deployment::ComponentLoader::Instance()->setComponentPath(compPath);
      deployment::ComponentLoader::Instance()->import("");
	  return true;
    }

    bool DeploymentComponent::componentLoaded(TaskContext* c) { return true; }

    void DeploymentComponent::componentUnloaded(TaskContext* c) { }

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
                        if ( (*it)->getName() == "LoadLibrary" ) {
                            Property<std::string> importp = *it;
                            if ( !importp.ready() ) {
                                log(Error)<< "Found 'LoadLibrary' statement, but it is not of type='string'."<<endlog();
                                valid = false;
                                continue;
                            }
                            if ( this->loadLibrary( importp.get() ) == false )
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
                                if ( nm.rvalue().getType() == "PeriodicActivity" || nm.rvalue().getType() == "Activity") {
                                    Property<double> per("Period","",0.0); // sets default to 0.0 in case of Activity.
                                    if (nm.rvalue().getProperty<double>("Period") )
                                        per = nm.rvalue().getProperty<double>("Period"); // work around RTT 1.0.2 bug.
                                    if ( !per.ready() && nm.rvalue().getType() == "PeriodicActivity") {
                                        log(Error)<<"Please specify period as <double> for "<< nm.rvalue().getType() <<endlog();
                                        valid = false;
                                    }
                                    Property<int> prio;
                                    if ( nm.rvalue().getProperty<int>("Priority") )
                                        prio = nm.rvalue().getProperty<int>("Priority"); // work around RTT 1.0.2 bug
                                    if ( !prio.ready() ) {
                                        log(Error)<<"Please specify priority as <short> for "<< nm.rvalue().getType()<<endlog();
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
                                        this->setNamedActivity(comp.getName(), nm.rvalue().getType(), per.get(), prio.get(), scheduler );
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
                                            this->setNamedActivity(comp.getName(), nm.rvalue().getType(), 0.0, prio.get(), scheduler );
                                        }
                                    } else
                                        if ( nm.rvalue().getType() == "SlaveActivity" ) {
                                            double period = 0.0;
                                            string master;
                                            if ( nm.rvalue().getProperty<string>("Master") ) {
                                                master = nm.rvalue().getProperty<string>("Master")->get();
                                                if (valid) {
                                                    this->setNamedActivity(comp.getName(), nm.rvalue().getType(), period, 0, 0, master );
                                                }
                                            } else {
                                                // No master given.
                                                if ( nm.rvalue().getProperty<double>("Period") )
                                                    period = nm.rvalue().getProperty<double>("Period")->get();
                                                if (valid) {
                                                    this->setNamedActivity(comp.getName(), nm.rvalue().getType(), period, 0, 0 );
                                                }
                                            }
                                        } else
                                            if ( nm.rvalue().getType() == "SequentialActivity" ) {
                                                this->setNamedActivity(comp.getName(), nm.rvalue().getType(), 0, 0, 0 );
                                            } else {
                                                log(Error) << "Unknown activity type: " << nm.rvalue().getType()<<endlog();
                                                valid = false;
                                            }
                            }
                        } else {
                            // no 'Activity' element, default to Slave:
                            //this->setNamedActivity(comp.getName(), "SlaveActivity", 0.0, 0, 0 );
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
                        comps[ comp.getName() ].loadedProperties = true;
                    }
                }
            }

            // Attach activities
            if ( comps[comp.getName()].act ) {
                if ( peer->getActivity() ) {
                    log(Info) << "Re-setting activity of "<< comp.getName() <<endlog();
                } else {
                    log(Info) << "Setting activity of "<< comp.getName() <<endlog();
                }
                peer->setActivity( comps[comp.getName()].act );
                assert( peer->engine()->getActivity() == comps[comp.getName()].act );
                comps[comp.getName()].act = 0; // drops ownership.
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

        // Finally, report success/failure (but ignore components that are actually running, as
        // they will have been configured/started previously)
        if (!valid) {
            for ( CompList::iterator cit = comps.begin(); cit != comps.end(); ++cit) {
                ComponentData* cd = &(cit->second);
                if ( cd->loaded && cd->autoconf &&
                     (cd->instance->getTaskState() != TaskCore::Stopped) &&
                     (cd->instance->getTaskState() != TaskCore::Running))
                    log(Error) << "Failed to configure component "<< cd->instance->getName()
                               << ": state is " << cd->instance->getTaskState() <<endlog();
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

            // only start if not already running (peer may have been previously
            // loaded/configured/started from the site deployer file)
            if (peer->isRunning()) 
            {
                continue;
            }

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
                        if (it->loadedProperties) {
                            string file = it->configfile; // get file name
                            PropertyLoader pl;
                            bool ret = pl.save( file, it->instance, true ); // save all !
                            if (!ret) {
                                log(Error) << "Failed to save properties for component "<< it->instance->getName() <<endlog();
                                valid = false;
                            } else {
                                log(Info) << "Saved Properties of "<< it->instance->getName() << " to "<<file<<endlog();
                            }
                        } else {
                            log(Info) << "Refusing to save property file that was not loaded for "<< it->instance->getName() <<endlog();
                        }
                    } else if (it->autosave) {
		      log(Error) << "AutoSave set but no property file specified. Specify one using the UpdateProperties simple element."<<endlog();
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
        while ( comps.size() > 0 && valid)
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

  bool DeploymentComponent::import(const std::string& path)
  {
    RTT::Logger::In in("DeploymentComponent::import");
    log(Debug) << "Importing Components, plugins and typekits from " <<  path << endlog();
    PluginLoader::Instance()->loadTypekits(path);
    PluginLoader::Instance()->loadPlugins(path);
    deployment::ComponentLoader::Instance()->import( path );
    return true;
  }

  bool DeploymentComponent::loadLibrary(const std::string& name)
  {
    RTT::Logger::In in("DeploymentComponent::loadLibrary");
    return PluginLoader::Instance()->loadTypekit(name,".") || PluginLoader::Instance()->loadPlugin(name,".") || deployment::ComponentLoader::Instance()->import(name, ".");
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


        TaskContext* instance = deployment::ComponentLoader::Instance()->loadComponent(name, type);

        if (!instance) {
	  return false;
        }

        // we need to set instance such that componentLoaded can lookup 'instance' in 'comps'
        comps[name].instance = instance;

        if (!this->componentLoaded( instance ) ) {
            log(Error) << "This deployer type refused to connect to "<< instance->getName() << ": aborting !" << endlog(Error);
            comps[name].instance = 0;
            deployment::ComponentLoader::Instance()->unloadComponent( instance );
            return false;
        }

        // unlikely that this fails (checked at entry)!
        this->addPeer( instance );
        log(Info) << "Adding "<< instance->getName() << " as new peer:  OK."<< endlog(Info);

        comps[name].loaded = true;

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
                    // allow subclasses to do cleanup too.
                    componentUnloaded( it->instance );
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
		deployment::ComponentLoader::Instance()->unloadComponent( it->instance );
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

    bool DeploymentComponent::setActivity(const std::string& comp_name,
                                          double period, int priority,
                                          int scheduler)
    {
        if ( this->setNamedActivity(comp_name, "Activity", period, priority, scheduler) ) {
            assert( comps[comp_name].instance );
            assert( comps[comp_name].act );
            comps[comp_name].instance->setActivity( comps[comp_name].act );
            comps[comp_name].act = 0;
            return true;
        }
        return false;
    }

    bool DeploymentComponent::setPeriodicActivity(const std::string& comp_name,
                                                  double period, int priority,
                                                  int scheduler)
    {
        if ( this->setNamedActivity(comp_name, "PeriodicActivity", period, priority, scheduler) ) {
            assert( comps[comp_name].instance );
            assert( comps[comp_name].act );
            comps[comp_name].instance->setActivity( comps[comp_name].act );
            comps[comp_name].act = 0;
            return true;
        }
        return false;
    }

    bool DeploymentComponent::setNonPeriodicActivity(const std::string& comp_name,
                                                     int priority,
                                                     int scheduler)
    {
        if ( this->setNamedActivity(comp_name, "NonPeriodicActivity", 0.0, priority, scheduler) ) {
            assert( comps[comp_name].instance );
            assert( comps[comp_name].act );
            comps[comp_name].instance->setActivity( comps[comp_name].act );
            comps[comp_name].act = 0;
            return true;
        }
        return false;
    }

    bool DeploymentComponent::setSlaveActivity(const std::string& comp_name,
                                               double period)
    {
        if ( this->setNamedActivity(comp_name, "SlaveActivity", period, 0, ORO_SCHED_OTHER ) ) {
            assert( comps[comp_name].instance );
            assert( comps[comp_name].act );
            comps[comp_name].instance->setActivity( comps[comp_name].act );
            comps[comp_name].act = 0;
            return true;
        }
        return false;
    }

    bool DeploymentComponent::setSequentialActivity(const std::string& comp_name)
    {
        if ( this->setNamedActivity(comp_name, "SequentialActivity", 0, 0, 0 ) ) {
            assert( comps[comp_name].instance );
            assert( comps[comp_name].act );
            comps[comp_name].instance->setActivity( comps[comp_name].act );
            comps[comp_name].act = 0;
            return true;
        }
        return false;
    }

    bool DeploymentComponent::setMasterSlaveActivity(const std::string& master,
                                                   const std::string& slave)
    {
        if ( this->setNamedActivity(slave, "SlaveActivity", 0, 0, ORO_SCHED_OTHER, master ) ) {
            assert( comps[slave].instance );
            assert( comps[slave].act );
            comps[slave].instance->setActivity( comps[slave].act );
            comps[slave].act = 0;
            return true;
        }
        return false;
    }


    bool DeploymentComponent::setNamedActivity(const std::string& comp_name,
                                               const std::string& act_type,
                                               double period, int priority,
                                               int scheduler, const std::string& master_name)
    {
        // This helper function does not actualy set the activity, it just creates it and
        // stores it in comps[comp_name].act
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
                else
                    if (act_type == "Activity") {
                        newact = new Activity(scheduler, priority, period, 0, comp_name);
                    }
                    else
                        if (act_type == "SequentialActivity") {
                            newact = new SequentialActivity();
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
                    if ( (*it)->getName() == "LoadLibrary" ) continue;
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
                log(Error)<< "Uncaught exception in kickOut() !"<< endlog();
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
