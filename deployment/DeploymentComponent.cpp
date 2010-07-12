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
#include "ComponentLoader.hpp"
#include <rtt/extras/Activities.hpp>
#include <rtt/extras/SequentialActivity.hpp>
#include <rtt/marsh/PropertyMarshaller.hpp>
#include <rtt/marsh/PropertyDemarshaller.hpp>
#include <rtt/Method.hpp>
#include <rtt/scripting/Scripting.hpp>
#include <rtt/ConnPolicy.hpp>
#include <rtt/plugin/PluginLoader.hpp>

#include <cstdio>
#include <cstdlib>

#include "ocl/Component.hpp"
#include <rtt/marsh/PropertyLoader.hpp>

#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <set>

using namespace Orocos;

namespace OCL
{
    using namespace std;
    using namespace RTT;
    using namespace RTT::marsh;
    using namespace RTT::detail;

    /**
     * I'm using a set to speed up lookups.
     */
    static std::set<string> valid_names;

#define ORO_str(s) ORO__str(s)
#define ORO__str(s) #s

    DeploymentComponent::DeploymentComponent(std::string name, std::string siteFile)
        : RTT::TaskContext(name, Stopped),
          autoUnload("AutoUnload",
                     "Stop, cleanup and unload all components loaded by the DeploymentComponent when it is destroyed.",
                     true),
          validConfig("Valid", false),
          sched_RT("ORO_SCHED_RT", ORO_SCHED_RT ),
          sched_OTHER("ORO_SCHED_OTHER", ORO_SCHED_OTHER ),
          lowest_Priority("LowestPriority", RTT::os::LowestPriority ),
          highest_Priority("HighestPriority", RTT::os::HighestPriority ),
          target("Target",
                 "The Orocos Target component suffix. Will be used in import statements to find matching components. Only change this if you know what you are doing.",
                 ORO_str(OROCOS_TARGET) )
    {
        this->addProperty( "RTT_COMPONENT_PATH", compPath ).doc("Locations to look for components. Use a colon or semi-colon separated list of paths. Defaults to the environment variable with the same name.");
        this->addProperty( autoUnload );
        this->addProperty( target );

        this->addAttribute( validConfig );
        this->addAttribute( sched_RT );
        this->addAttribute( sched_OTHER );
        this->addAttribute( lowest_Priority );
        this->addAttribute( highest_Priority );


        this->addOperation("loadLibrary", &DeploymentComponent::loadLibrary, this, ClientThread).doc("Load a new library into memory.").arg("Name", "The name of the to be loaded library.");
        this->addOperation("import", &DeploymentComponent::import, this, ClientThread).doc("Load all libraries in Path into memory.").arg("Path", "The name of the directory where libraries are located.");


        this->addOperation("loadComponent", &DeploymentComponent::loadComponent, this, ClientThread).doc("Load a new component instance from a library.").arg("Name", "The name of the to be created component").arg("Type", "The component type, used to lookup the library.");
        this->addOperation("unloadComponent", &DeploymentComponent::unloadComponent, this, ClientThread).doc("Unload a loaded component instance.").arg("Name", "The name of the to be created component");
        this->addOperation("displayComponentTypes", &DeploymentComponent::displayComponentTypes, this, ClientThread).doc("Print out a list of all component types this component can create.");

        this->addOperation("loadConfiguration", &DeploymentComponent::loadConfiguration, this, ClientThread).doc("Load a new XML configuration from a file (identical to loadComponents).").arg("File", "The file which contains the new configuration.");
        this->addOperation("loadConfigurationString", &DeploymentComponent::loadConfigurationString, this, ClientThread).doc("Load a new XML configuration from a string.").arg("Text", "The string which contains the new configuration.");
        this->addOperation("clearConfiguration", &DeploymentComponent::clearConfiguration, this, ClientThread).doc("Clear all configuration settings.");

        this->addOperation("loadComponents", &DeploymentComponent::loadComponents, this, ClientThread).doc("Load components listed in an XML configuration file.").arg("File", "The file which contains the new configuration.");
        this->addOperation("configureComponents", &DeploymentComponent::configureComponents, this, ClientThread).doc("Apply a loaded configuration to the components and configure() them if AutoConf is set.");
        this->addOperation("startComponents", &DeploymentComponent::startComponents, this, ClientThread).doc("Start the components configured for AutoStart.");
        this->addOperation("stopComponents", &DeploymentComponent::stopComponents, this, ClientThread).doc("Stop all the configured components (with or without AutoStart).");
        this->addOperation("cleanupComponents", &DeploymentComponent::cleanupComponents, this, ClientThread).doc("Cleanup all the configured components (with or without AutoConf).");
        this->addOperation("unloadComponents", &DeploymentComponent::unloadComponents, this, ClientThread).doc("Unload all the previously loaded components.");

        this->addOperation("kickStart", &DeploymentComponent::kickStart, this, ClientThread).doc("Calls loadComponents, configureComponents and startComponents in a row.").arg("File", "The file which contains the XML configuration to use.");
        this->addOperation("kickOutAll", &DeploymentComponent::kickOutAll, this, ClientThread).doc("Calls stopComponents, cleanupComponents and unloadComponents in a row.");

        this->addOperation("kickOutComponent", &DeploymentComponent::kickOutComponent, this, ClientThread).doc("Calls stopComponents, cleanupComponent and unloadComponent in a row.").arg("comp_name", "component name");
        this->addOperation("kickOut", &DeploymentComponent::kickOut, this, ClientThread).doc("Calls stopComponents, cleanupComponents and unloadComponents in a row.").arg("File", "The file which contains the name of the components to kickOut (for example, the same used in kickStart).");

        // Work around compiler ambiguity:
        typedef bool(DeploymentComponent::*DCFun)(const std::string&, const std::string&);
        DCFun cp = &DeploymentComponent::connectPeers;
        this->addOperation("connectPeers", cp, this, ClientThread).doc("Connect two Components known to this Component.").arg("One", "The first component.").arg("Two", "The second component.");
        cp = &DeploymentComponent::connectPorts;
        this->addOperation("connectPorts", cp, this, ClientThread).doc("Connect the Data Ports of two Components known to this Component.").arg("One", "The first component.").arg("Two", "The second component.");
        cp = &DeploymentComponent::addPeer;
        typedef bool(DeploymentComponent::*DC4Fun)(const std::string&, const std::string&,
                                                   const std::string&, const std::string&);
        DC4Fun cp4 = &DeploymentComponent::connectPorts;
        this->addOperation("connectTwoPorts", cp4, this, ClientThread).doc("Connect two ports of Components known to this Component.")
                .arg("One", "The first component.")
                .arg("PortOne", "The port name of the first component.")
                .arg("Two", "The second component.")
                .arg("PortTwo", "The port name of the second component.");

        this->addOperation("connectServices", &DeploymentComponent::connectServices, this, ClientThread).doc("Connect the matching provides/requires services of two Components known to this Component.").arg("One", "The first component.").arg("Two", "The second component.");

        this->addOperation("addPeer", cp, this, ClientThread).doc("Add a peer to a Component.").arg("From", "The first component.").arg("To", "The other component.");
        typedef void(DeploymentComponent::*RPFun)(const std::string&);
        RPFun rp = &RTT::TaskContext::removePeer;
        this->addOperation("removePeer", rp, this, ClientThread).doc("Remove a peer from this Component.").arg("PeerName", "The name of the peer to remove.");

        this->addOperation("setActivity", &DeploymentComponent::setActivity, this, ClientThread).doc("Attach an activity to a Component.").arg("CompName", "The name of the Component.").arg("Period", "The period of the activity (set to 0.0 for non periodic).").arg("Priority", "The priority of the activity.").arg("SchedType", "The scheduler type of the activity.");
        this->addOperation("setPeriodicActivity", &DeploymentComponent::setPeriodicActivity, this, ClientThread).doc("Attach a periodic activity to a Component.").arg("CompName", "The name of the Component.").arg("Period", "The period of the activity.").arg("Priority", "The priority of the activity.").arg("SchedType", "The scheduler type of the activity.");
        this->addOperation("setSequentialActivity", &DeploymentComponent::setSequentialActivity, this, ClientThread).doc("Attach a 'stand alone' sequential activity to a Component.").arg("CompName", "The name of the Component.");
        this->addOperation("setSlaveActivity", &DeploymentComponent::setSlaveActivity, this, ClientThread).doc("Attach a 'stand alone' slave activity to a Component.").arg("CompName", "The name of the Component.").arg("Period", "The period of the activity (set to zero for non periodic).");
        this->addOperation("setMasterSlaveActivity", &DeploymentComponent::setMasterSlaveActivity, this, ClientThread).doc("Attach a slave activity with a master to a Component. The slave becomes a peer of the master as well.").arg("Master", "The name of the Component which is master of the Slave.").arg("Slave", "The name of the Component which gets the SlaveActivity.");

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
        Logger::In in("DeploymentComponent::configure");
        char* paths = getenv("RTT_COMPONENT_PATH");
        if (compPath.empty() )
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
        ComponentLoader::Instance()->setComponentPath(compPath);
        ComponentLoader::Instance()->import("");
        return true;
    }

    bool DeploymentComponent::componentLoaded(RTT::TaskContext* c) { return true; }

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
        RTT::Logger::In in("DeploymentComponent::connectPeers");
        RTT::TaskContext* t1 = this->getPeer(one);
        RTT::TaskContext* t2 = this->getPeer(other);
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
        RTT::Logger::In in("DeploymentComponent::addPeer");
        RTT::TaskContext* t1 = this->getPeer(from);
        RTT::TaskContext* t2 = this->getPeer(to);
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
	RTT::Logger::In in("DeploymentComponent::connectPorts");
        RTT::TaskContext* a, *b;
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
	RTT::Logger::In in("DeploymentComponent::connectPorts");
        RTT::TaskContext* a, *b;
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
        base::PortInterface* ap, *bp;
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
            if ( false ) {
                log(Info) << "Port '"<< ap->getName() << "' of Component '"<<a->getName()
                          << "' is already connected to port '"<< bp->getName() << "' of Component '"<<b->getName()<<"'."<<endlog();
                return true;
            }
            log(Error) << "Port '"<< ap->getName() << "' of Component '"<<a->getName()
                       << "' and port '"<< bp->getName() << "' of Component '"<<b->getName()
                       << "' are already connected but (probably) not to each other."<<endlog();
            return false;
        }

        // use the base::PortInterface implementation
        if ( ap->connectTo( bp ) ) {
            // all went fine.
            log(Info)<< "Connected Port " << ap->getName() << " to peer Task "<< b->getName() <<"." << endlog();
            return true;
        } else {
            log(Error)<< "Failed to connect Port " << ap->getName() << " to peer Task "<< b->getName() <<"." << endlog();
            return true;
        }
    }

    bool DeploymentComponent::connectServices(const std::string& one, const std::string& other)
    {
    RTT::Logger::In in("DeploymentComponent::connectServices");
        RTT::TaskContext* a, *b;
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

        return a->connectServices(b);
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
        RTT::Logger::In in("DeploymentComponent::loadComponents");

        RTT::PropertyBag from_file;
        log(Info) << "Loading '" <<configurationfile<<"'."<< endlog();
        // demarshalling failures:
        bool failure = false;
        // semantic failures:
        bool valid = validConfig.get();
        marsh::PropertyDemarshaller demarshaller(configurationfile);
        try {
            if ( demarshaller.deserialize( from_file ) )
                {
                    valid = true;
                    log(Info)<<"Validating new configuration..."<<endlog();
                    if ( from_file.empty() ) {
                        log(Error)<< "Configuration was empty !" <<endlog();
                        valid = false;
                    }

                    //for (RTT::PropertyBag::Names::iterator it= nams.begin();it != nams.end();it++) {
                    for (RTT::PropertyBag::iterator it= from_file.begin(); it!=from_file.end();it++) {
                        // Read in global options.
                        if ( (*it)->getName() == "Import" ) {
                            RTT::Property<std::string> importp = *it;
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
                            RTT::Property<std::string> includep = *it;
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
                        RTT::Property<RTT::PropertyBag> comp = *it;
                        if ( !comp.ready() ) {
                            log(Error)<< "RTT::Property '"<< *it <<"' should be a struct, Include or Import statement." << endlog();
                            valid = false;
                            continue;
                        }

                        //Check if it is a ConnPolicy
                        // convert to Property<ConnPolicy>
                        Property<ConnPolicy> cp_prop((*it)->getName(),"");
                        assert( cp_prop.ready() );
                        if ( cp_prop.compose( comp ) ) {
                            //It's a connection policy.
                            conmap[cp_prop.getName()].policy = cp_prop.get();
                            log(Debug) << "Saw connection policy " << (*it)->getName() << endlog();
                            continue;
                        }

                        // Parse the options before creating the component:
                        for (RTT::PropertyBag::const_iterator optit= comp.rvalue().begin(); optit != comp.rvalue().end();optit++) {
                            if ( valid_names.find( (*optit)->getName() ) == valid_names.end() ) {
                                log(Error) << "Unknown type syntax: '"<< (*optit)->getName() << "' in component struct "<< comp.getName() <<endlog();
                                valid = false;
                                continue;
                            }
                            if ( (*optit)->getName() == "AutoConnect" ) {
                                RTT::Property<bool> ps = comp.rvalue().getProperty("AutoConnect");
                                if (!ps.ready()) {
                                    log(Error) << "AutoConnect must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].autoconnect = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "AutoStart" ) {
                                RTT::Property<bool> ps = comp.rvalue().getProperty("AutoStart");
                                if (!ps.ready()) {
                                    log(Error) << "AutoStart must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].autostart = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "AutoSave" ) {
                                RTT::Property<bool> ps = comp.rvalue().getProperty("AutoSave");
                                if (!ps.ready()) {
                                    log(Error) << "AutoSave must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].autosave = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "AutoConf" ) {
                                RTT::Property<bool> ps = comp.rvalue().getProperty("AutoConf");
                                if (!ps.ready()) {
                                    log(Error) << "AutoConf must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].autoconf = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "Server" ) {
                                RTT::Property<bool> ps = comp.rvalue().getProperty("Server");
                                if (!ps.ready()) {
                                    log(Error) << "Server must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].server = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "UseNamingService" ) {
                                RTT::Property<bool> ps = comp.rvalue().getProperty("UseNamingService");
                                if (!ps.ready()) {
                                    log(Error) << "UseNamingService must be of type <boolean>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].use_naming = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "PropertyFile" ) {
                                RTT::Property<string> ps = comp.rvalue().getProperty("PropertyFile");
                                if (!ps.ready()) {
                                    log(Error) << "PropertyFile must be of type <string>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].configfile = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "UpdateProperties" ) {
                                RTT::Property<string> ps = comp.rvalue().getProperty("UpdateProperties");
                                if (!ps.ready()) {
                                    log(Error) << "UpdateProperties must be of type <string>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].configfile = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "LoadProperties" ) {
                                RTT::Property<string> ps = comp.rvalue().getProperty("LoadProperties");
                                if (!ps.ready()) {
                                    log(Error) << "LoadProperties must be of type <string>" << endlog();
                                    valid = false;
                                } else
                                    comps[comp.getName()].configfile = ps.get();
                                continue;
                            }
                            if ( (*optit)->getName() == "Properties" ) {
                                base::PropertyBase* ps = comp.rvalue().getProperty("Properties");
                                if (!ps) {
                                    log(Error) << "Properties must be a <struct>" << endlog();
                                    valid = false;
                                }
                                continue;
                            }
                            if ( (*optit)->getName() == "ProgramScript" ) {
                                base::PropertyBase* ps = comp.rvalue().getProperty("ProgramScript");
                                if (!ps) {
                                    log(Error) << "ProgramScript must be of type <string>" << endlog();
                                    valid = false;
                                }
                                continue;
                            }
                            if ( (*optit)->getName() == "StateMachineScript" ) {
                                base::PropertyBase* ps = comp.rvalue().getProperty("StateMachineScript");
                                if (!ps) {
                                    log(Error) << "StateMachineScript must be of type <string>" << endlog();
                                    valid = false;
                                }
                                continue;
                            }
                        }

                        // Check if we know or are this component.
                        RTT::TaskContext* c = 0;
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
                        if ( comp.get().getProperty("PropFile") )  // PropFile is deprecated
                            comp.get().getProperty("PropFile")->setName("PropertyFile");

                        // connect ports 'Ports' tag is optional.
                        RTT::Property<RTT::PropertyBag>* ports = comp.get().getPropertyType<PropertyBag>("Ports");
                        if ( ports != 0 ) {
                            RTT::PropertyBag::Names pnams = ports->get().list();
                            for (RTT::PropertyBag::Names::iterator pit= pnams.begin(); pit !=pnams.end(); pit++) {
                                base::PortInterface* p = c->ports()->getPort(*pit);
                                if ( !p ) {
                                    log(Error)<< "Component '"<< c->getName() <<"' does not have a Port '"<<*pit<<"'." << endlog();
                                    valid = false;
                                }
                                if ( ports->get().getProperty(*pit) == 0) {
                                    log(Error)<< "RTT::Property '"<< *pit <<"' is not of type 'string'." << endlog();
                                    valid = false;
                                }
                                // store the port
                                if (valid){
                                    string port_name = ports->get().getPropertyType<string>(*pit)->get();
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
                            RTT::Property<RTT::PropertyBag> nm = comp.value().find("Peers");
                            if ( !nm.ready() ) {
                                log(Error)<<"RTT::Property 'Peers' must be a 'struct', was type "<< comp.value().find("Peers")->getType() << endlog();
                                valid = false;
                            } else {
                                for (RTT::PropertyBag::const_iterator it= nm.rvalue().begin(); it != nm.rvalue().end();it++) {
                                    RTT::Property<std::string> pr = *it;
                                    if ( !pr.ready() ) {
                                        log(Error)<<"RTT::Property 'Peer' does not have type 'string'."<<endlog();
                                        valid = false;
                                        continue;
                                    }
                                }
                            }
                        }

                        // Read the activity profile if present.
                        if ( comp.value().find("Activity") != 0) {
                            RTT::Property<RTT::PropertyBag> nm = comp.value().find("Activity");
                            if ( !nm.ready() ) {
                                log(Error)<<"RTT::Property 'Activity' must be a 'struct'."<<endlog();
                                valid = false;
                            } else {
                                if ( nm.rvalue().getType() == "PeriodicActivity" ) {
                                    RTT::Property<double> per = nm.rvalue().getProperty("Period"); // work around RTT 1.0.2 bug.
                                    if ( !per.ready() ) {
                                        log(Error)<<"Please specify period <double> of PeriodicActivity."<<endlog();
                                        valid = false;
                                    }
                                    RTT::Property<int> prio = nm.rvalue().getProperty("Priority"); // work around RTT 1.0.2 bug
                                    if ( !prio.ready() ) {
                                        log(Error)<<"Please specify priority <short> of PeriodicActivity."<<endlog();
                                        valid = false;
                                    }
                                    RTT::Property<string> sched;
                                    if (nm.rvalue().getProperty("Scheduler") )
                                        sched = nm.rvalue().getProperty("Scheduler"); // work around RTT 1.0.2 bug
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
                                    if ( nm.rvalue().getType() == "Activity" || nm.rvalue().getType() == "NonPeriodicActivity" ) {
                                        RTT::Property<double> per = nm.rvalue().getProperty("Period");
                                        if ( !per.ready() ) {
                                            per = Property<double>("p","",0.0); // default to 0.0
                                        }
                                        RTT::Property<int> prio = nm.rvalue().getProperty("Priority");
                                        if ( !prio.ready() ) {
                                            log(Error)<<"Please specify priority <short> of Activity."<<endlog();
                                            valid = false;
                                        }
                                        RTT::Property<string> sched = nm.rvalue().getProperty("Scheduler");
                                        int scheduler = ORO_SCHED_RT;
                                        if ( sched.ready() ) {
                                            int scheduler = string_to_oro_sched( sched.get());
                                            if (scheduler == -1 )
                                                valid = false;
                                        }
                                        if (valid) {
                                            this->setNamedActivity(comp.getName(), nm.rvalue().getType(), per.get(), prio.get(), scheduler );
                                        }
                                    } else
                                        if ( nm.rvalue().getType() == "SlaveActivity" ) {
                                            double period = 0.0;
                                            string master;
                                            if ( nm.rvalue().getProperty("Master") ) {
                                                master = nm.rvalue().getPropertyType<string>("Master")->get();
                                                if (valid) {
                                                    this->setNamedActivity(comp.getName(), nm.rvalue().getType(), period, 0, 0, master );
                                                }
                                            } else {
                                                // No master given.
                                                if ( nm.rvalue().getProperty("Period") )
                                                    period = nm.rvalue().getPropertyType<double>("Period")->get();
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
                            //this->setNamedActivity(comp.getName(), "extras::SlaveActivity", 0.0, 0, 0 );
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
        RTT::Logger::In in("DeploymentComponent::configureComponents");
        if ( root.empty() ) {
            RTT::Logger::log() << RTT::Logger::Error
                          << "No components loaded by DeploymentComponent !" <<endlog();
            return false;
        }

        bool valid = true;

        // Connect peers
        for (RTT::PropertyBag::iterator it= root.begin(); it!=root.end();it++) {

            RTT::Property<RTT::PropertyBag> comp = *it;

            RTT::TaskContext* peer = comps[ comp.getName() ].instance;
            if ( !peer ) {
                log(Error) << "Peer not found: "<< comp.getName() <<endlog();
                valid=false;
                continue;
            }

            comps[comp.getName()].instance = peer;

            // Setup the connections from each component to the
            // others.
            RTT::Property<RTT::PropertyBag> peers = comp.rvalue().find("Peers");
            if ( peers.ready() )
                for (RTT::PropertyBag::const_iterator it= peers.rvalue().begin(); it != peers.rvalue().end();it++) {
                    RTT::Property<string> nm = (*it);
                    if ( nm.ready() )
                        {
                            this->addPeer( comps[comp.getName()].instance->getName(), nm.value() );
                            log(Info) << this->getName() << " connects " <<
                                comps[comp.getName()].instance->getName() << " to "<< nm.value()  << endlog();
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
                log(Warning) << "Can not form connection with topic "<<connection_name<<" with only one Port from "<< connection->owners[0]->getName() << endlog();
                continue;
            }
            // first find all write ports.
            base::PortInterface* writer = 0;
            ConnectionData::Ports::iterator p = connection->ports.begin();

            // If one of the ports is connected, use that one as writer to connect to.
            vector<OutputPortInterface*> writers;
            while (p !=connection->ports.end() ) {
                if ( OutputPortInterface* out = dynamic_cast<base::OutputPortInterface*>( *p ) ) {
                    if ( writer ) {
                        log(Info) << "Forming multi-output connections with additional OutputPort " << (*p)->getName() << "."<<endlog();
                    } else
                        writer = *p;
                    writers.push_back( out );
                    std::string owner = it->second.owners[p - it->second.ports.begin()]->getName();
                    log(Info) << "Component "<< owner << "'s OutputPort "<< writer->getName()<< " will write topic "<<it->first<< endlog();
                }
                ++p;
            }

            // Inform the user of non-optimal connections:
            if ( writer == 0 ) {
                log(Error) << "No OutputPort listed that writes " << it->first << endlog();
                valid = false;
                break;
            }

            // connect all ports to writer
            p = connection->ports.begin();
            vector<OutputPortInterface*>::iterator w = writers.begin();

            while (w != writers.end() ) {
                while (p != connection->ports.end() ) {
                    // connect all readers to the list of writers
                    if ( dynamic_cast<base::InputPortInterface*>( *p ) )
                    {
                        string owner = connection->owners[p - connection->ports.begin()]->getName();
                        // only try to connect p if it is not in the same connection of writer.
                        // OK. p is definately no part of writer's connection. Try to connect and flag errors if it fails.
                        if ( (*w)->connectTo( *p, connection->policy ) == false) {
                            log(Error) << "Could not subscribe InputPort "<< owner<<"."<< (*p)->getName() << " to topic " << (*w)->getName() <<'/'<< connection_name <<endlog();
                            valid = false;
                        } else {
                            log(Info) << "Subscribed InputPort "<< owner<<"."<< (*p)->getName() <<" to topic " << (*w)->getName() <<'/'<< connection_name <<endlog();
                        }
                    }
                    ++p;
                }
                ++w;
                p = connection->ports.begin();
            }
        }

        // Autoconnect ports. The port name is the topic name.
        for (RTT::PropertyBag::iterator it= root.begin(); it!=root.end();it++) {
            RTT::Property<RTT::PropertyBag> comp = *it;
            if ( !comp.ready() )
                continue;

            RTT::TaskContext* peer = comps[ comp.getName() ].instance;

            // only autoconnect if AutoConnect == 1 and peer has AutoConnect == 1
            // There should only be one writer; more than one will lead to undefined behaviour.
            // reader<->reader connections will silently fail and be retried once a writer is found.
            if ( comps[comp.getName()].autoconnect ) {
                // XXX/TODO This is broken: we should not rely on the peers to implement AutoConnect!
                RTT::TaskContext::PeerList peers = peer->getPeerList();
                for(RTT::TaskContext::PeerList::iterator pit = peers.begin(); pit != peers.end(); ++pit) {
                    if ( comps.count( *pit ) && comps[ *pit ].autoconnect ) {
                        RTT::TaskContext* other = peer->getPeer( *pit );
                        ::RTT::connectPorts( peer, other );
                    }
                }
            }
        }

        // Main configuration
        for (RTT::PropertyBag::iterator it= root.begin(); it!=root.end();it++) {

            RTT::Property<RTT::PropertyBag> comp = *it;
            RTT::Property<string> dummy;
            RTT::TaskContext* peer = comps[ comp.getName() ].instance;

            // do not configure when not stopped.
            if ( peer->getTaskState() > Stopped) {
                log(Warning) << "Component "<< peer->getName()<< " doesn't need to be configured (already Running)." <<endlog();
                continue;
            }

            // Check for default properties to set.
            for (RTT::PropertyBag::const_iterator pf = comp.rvalue().begin(); pf!= comp.rvalue().end(); ++pf) {
                // set PropFile name if present
                if ( (*pf)->getName() == "Properties"){
                    RTT::Property<RTT::PropertyBag> props = *pf; // convert to type.
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
            for (RTT::PropertyBag::const_iterator pf = comp.rvalue().begin(); pf!= comp.rvalue().end(); ++pf) {
                // set PropFile name if present
                if ( (*pf)->getName() == "PropertyFile" || (*pf)->getName() == "UpdateProperties" || (*pf)->getName() == "LoadProperties"){
                    dummy = *pf; // convert to type.
                    string filename = dummy.get();
                    marsh::PropertyLoader pl;
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
            for (RTT::PropertyBag::const_iterator ps = comp.rvalue().begin(); ps!= comp.rvalue().end(); ++ps) {
                RTT::Property<string> pscript;
                if ( (*ps)->getName() == "ProgramScript" )
                    pscript = *ps;
                if ( pscript.ready() ) {
                    valid = valid && peer->getProvider<Scripting>("scripting")->loadPrograms( pscript.get() );
                }
                RTT::Property<string> sscript;
                if ( (*ps)->getName() == "StateMachineScript" )
                    sscript = *ps;
                if ( sscript.ready() ) {
                    valid = valid && peer->getProvider<Scripting>("scripting")->loadStateMachines( sscript.get() );
                }
            }

            // AutoConf
            if (comps[comp.getName()].autoconf )
                {
                    if( !peer->isRunning() )
                        {
                            if ( peer->configure() == false)
                                valid = false;
                        }
                    else
                        log(Warning) << "Apparently component "<< peer->getName()<< " don't need to be configured (already Running)." <<endlog();
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
        RTT::Logger::In in("DeploymentComponent::startComponents");
        if (validConfig.get() == false) {
            log(Error) << "Not starting components with invalid configuration." <<endlog();
            return false;
        }
        bool valid = true;
        for (RTT::PropertyBag::iterator it= root.begin(); it!=root.end();it++) {

            TaskContext* peer = comps[ (*it)->getName() ].instance;

            // only start if not already running (peer may have been previously
            // loaded/configured/started from the site deployer file)
            if (peer->isRunning())
            {
                continue;
            }

            // AutoStart
            if (comps[(*it)->getName()].autostart )
                if ( !peer || ( !peer->isRunning() && peer->start() == false) )
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
                if ( it->autostart && it->instance->getTaskState() != base::TaskCore::Running )
                    log(Error) << "Failed to start component "<< it->instance->getName() <<endlog();
            }
        } else {
            log(Info) << "Startup of 'AutoStart' components successful." <<endlog();
        }
        return valid;
    }

    bool DeploymentComponent::stopComponents()
    {
        RTT::Logger::In in("DeploymentComponent::stopComponents");
        bool valid = true;
        // 1. Stop all activities, give components chance to cleanup.
        for ( CompList::iterator cit = comps.begin(); cit != comps.end(); ++cit) {
            ComponentData* it = &(cit->second);
            if ( it->instance && !it->proxy ) {
                if ( !it->instance->isRunning() ||
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
        RTT::Logger::In in("DeploymentComponent::cleanupComponents");
        bool valid = true;
        // 1. Cleanup all activities, give components chance to cleanup.
        for ( CompList::iterator cit = comps.begin(); cit != comps.end(); ++cit) {
            ComponentData* it = &(cit->second);
            if (it->instance && !it->proxy) {
                if ( it->instance->getTaskState() <= base::TaskCore::Stopped ) {
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
        ComponentLoader::Instance()->import( path );
        return true;
    }

    bool DeploymentComponent::loadLibrary(const std::string& name)
    {
        RTT::Logger::In in("DeploymentComponent::loadLibrary");
        return ComponentLoader::Instance()->import(name, "");
    }

    // or type is a shared library or it is a class type.
    bool DeploymentComponent::loadComponent(const std::string& name, const std::string& type)
    {
        RTT::Logger::In in("DeploymentComponent::loadComponent");

        if ( type == "RTT::PropertyBag" )
            return false; // It should be present as peer.

        if ( this->getPeer(name) || ( comps.find(name) != comps.end() && comps[name].instance != 0) ) {
            log(Error) <<"Failed to load component with name "<<name<<": already present as peer or loaded."<<endlog();
            return false;
        }

        TaskContext* instance = ComponentLoader::Instance()->loadComponent(name, type);

        if (!instance) {
            return false;
        }

        if (!this->componentLoaded( instance ) ) {
            log(Error) << "This deployer type refused to connect to "<< instance->getName() << ": aborting !" << endlog(Error);
            ComponentLoader::Instance()->unloadComponent( instance );
            return false;
        }

        // unlikely that this fails (checked at entry)!
        this->addPeer( instance );
        log(Info) << "Adding "<< instance->getName() << " as new peer:  OK."<< endlog(Info);

        comps[name].instance = instance;
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
            if ( !it->instance->isRunning() ) {
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
                RTT::Property<RTT::PropertyBag>* pcomp = root.getPropertyType<PropertyBag>(name);
                if (pcomp) {
                    root.removeProperty(pcomp);
                }

                // Finally, delete the activity before the TC !
                delete it->act;
                it->act = 0;
                ComponentLoader::Instance()->unloadComponent( it->instance );
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
        RTT::TaskContext* peer = 0;
        base::ActivityInterface* master_act = 0;
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

        base::ActivityInterface* newact = 0;
        // standard case:
        if ( act_type == "Activity")
            newact = new RTT::Activity(scheduler, priority, period);
        else
            // special cases:
            if ( act_type == "PeriodicActivity" && period != 0.0)
                newact = new RTT::extras::PeriodicActivity(scheduler, priority, period);
            else
            if ( act_type == "NonPeriodicActivity" && period == 0.0)
                newact = new RTT::Activity(scheduler, priority);
            else
                if ( act_type == "SlaveActivity" ) {
                    if ( master_act == 0 )
                        newact = new extras::SlaveActivity(period);
                    else {
                        newact = new extras::SlaveActivity(master_act);
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
            log(Error) << "Can't create '"<< act_type << "' for component "<<comp_name<<": incorrect arguments."<<endlog();
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
        RTT::Logger::In in("DeploymentComponent");
        RTT::TaskContext* c;
        if ( name == this->getName() )
            c = this;
        else
            c = this->getPeer(name);
        if (!c) {
            log(Error)<<"No such peer to configure: "<<name<<endlog();
            return false;
        }

        marsh::PropertyLoader pl;
        return pl.configure( filename, c, true ); // strict:true
    }

    FactoryMap& DeploymentComponent::getFactories()
    {
        return ComponentFactories::Instance();
    }

    void DeploymentComponent::kickOut(const std::string& config_file)
    {
        RTT::Logger::In in("DeploymentComponent::kickOut");
        RTT::PropertyBag from_file;
        RTT::Property<std::string>  import_file;
        std::vector<std::string> deleted_components_type;

        // demarshalling failures:
        bool failure = false;

        marsh::PropertyDemarshaller demarshaller(config_file);
        try {
            if ( demarshaller.deserialize( from_file ) ){
                for (RTT::PropertyBag::iterator it= from_file.begin(); it!=from_file.end();it++) {
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
                log(Error)<< "Uncaught exception in kickOut() !"<< endlog();
                failure = true;
            }
    }

    bool DeploymentComponent::cleanupComponent(RTT::TaskContext *instance)
    {
        RTT::Logger::In in("DeploymentComponent::cleanupComponent");
        bool valid = true;
        // 1. Cleanup a single activities, give components chance to cleanup.
        if (instance) {
            if ( instance->getTaskState() <= base::TaskCore::Stopped ) {
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
        RTT::Logger::In in("DeploymentComponent::stopComponent");
        bool valid = true;

        if ( instance ) {
            if ( !instance->isRunning() ||
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
        RTT::Logger::In in("DeploymentComponent::kickOutComponent");
        base::PropertyBase *it = root.find( comp_name );
        if(!it) {
            log(Error) << "Peer "<< comp_name << " not found in RTT::PropertyBag root"<< endlog();
            return false;
        }

        RTT::TaskContext* peer = comps[ comp_name ].instance;

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
