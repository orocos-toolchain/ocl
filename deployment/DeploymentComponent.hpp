/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:34:31 CEST 2008  DeploymentComponent.hpp

                        DeploymentComponent.hpp -  description
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


#ifndef OCL_DEPLOYMENTCOMPONENT_HPP
#define OCL_DEPLOYMENTCOMPONENT_HPP

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/extras/Properties.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/InputPort.hpp>
#include <rtt/OutputPort.hpp>
#include <ocl/OCL.hpp>
#include <vector>
#include <map>
#include <rtt/marsh/PropertyDemarshaller.hpp>

// Suppress warnings in ocl/Component.hpp
#ifndef OCL_STATIC
#define OCL_STATIC
#include <ocl/Component.hpp>
#undef OCL_STATIC
#else
#include <ocl/Component.hpp>
#endif

namespace OCL
{

    /**
     * A Component for deploying (configuring) other components in an
     * application. It allows to create connections between components,
     * load the properties and scripts for components and setup
     * component activities.
     *
     * The main idea is to load an XML file as described in the
     * Deployment Component Manual. It dictates the libraries to load,
     * the components to create, configure and start. Every aspect of
     * the XML file can be expressed in a program script as well. If
     * you want this component to execute a program script, assign it
     * a periodic activity using either a 'site local' XML script (see below),
     * or by listing the DeploymentComponent in your main
     * XML file.
     *
     * @section sect-site-local Automatically loading a site local XML file
     * It is possible to store site local settings in a separate
     * XML configuration file which will be automatically loaded
     * when the DeploymentComponent is created. The default name of
     * this file is 'this->getName() + "-site.cpf"'. It is only looked
     * for and loaded in the constructor of this class.
     *
     * @section sect-conf-depl Configuring the DeploymentComponent itself.
     * When reading an XML file (for example, when using kickStart() or
     * loadComponents() ) the DeploymentComponent checks if a section
     * is devoted to itself by comparing the listed component name with its own
     * name ( this->getName() ). If it matches, it applies the configuration
     * instructions to itself in the same manner as it would configure
     * other components.
     *
     */
    class OCL_API DeploymentComponent
        : public RTT::TaskContext
    {
    protected:
        /**
         * This bag stores the current configuration.
         * It is the cumulation of all loadConfiguration() calls.
         */
        RTT::PropertyBag root;
        std::string compPath;
        RTT::Property<bool> autoUnload;
        RTT::Attribute<bool> validConfig;
        RTT::Constant<int> sched_RT;
        RTT::Constant<int> sched_OTHER;
        RTT::Constant<int> lowest_Priority;
        RTT::Constant<int> highest_Priority;
        RTT::Attribute<std::string> target;
        /// Next group number
        int nextGroup;

        /**
         * Each configured component is stored in a struct like this.
         * We need this to keep track of: 1. if we created an activity for it.
         * 2. if we loaded it.
         */
        struct ComponentData {
            ComponentData()
                : instance(0), act(0), loaded(false), loadedProperties(false),
                  autostart(false), autoconf(false),
                  autoconnect(false),  autosave(false),
                  proxy(false), server(false),
                  use_naming(true),
                  configfile(""),
                  group(0)
            {}
            /**
             * The component instance. This is always a valid pointer.
             */
            RTT::TaskContext* instance;
            /**
             * The activity created by DeploymentComponent.
             * May be null or an activity that we own.
             */
            base::ActivityInterface* act;
            /**
             * True if it was loaded and created by DeploymentComponent.
             * If true, instance may be deleted during
             * unloadComponent.
             */
            bool loaded;
            /**
             * True if successfully loaded a property file, and so
             *  will need auto-saving (it autosave is on)
             */
            bool loadedProperties;
            bool autostart, autoconf, autoconnect, autosave;
            bool proxy, server, use_naming;
            std::string configfile;
            std::vector<std::string> plugins;
            /// Group number this component belongs to
            int group;
        };

        /**
         * Assembles all ports which share a connection.
         */
        struct ConnectionData {
            typedef std::vector<RTT::base::PortInterface*> Ports;
            typedef std::vector<RTT::TaskContext*>   Owners;
            Ports ports;
            Owners owners;
            RTT::ConnPolicy policy;
        };

        /**
         * This maps connection names to associated ports.
         */
        typedef std::map<std::string, ConnectionData> ConMap;
        ConMap conmap;

        /**
         * This vector holds the dynamically loaded components.
         */
        typedef std::map<std::string, ComponentData> CompList;
        CompList comps;

        /**
         * This function imports available plugins from
         * the path formed by the expression
         *  @code ComponentPath + "/rtt/"+ Target + "/plugins" @endcode
         * @return always true.
         */
        bool configureHook();

        /**
         * This method removes all references to the component hold in \a cit,
         * on the condition that it is not running.
         */
        bool unloadComponentImpl( CompList::iterator cit );


        /**
         * Hook function for subclasses. Allows a subclass
         * to abort or extend the loading of a component.
         * By default, this function returns true.
         * @return false if the component should be unloaded again,
         * true if loading was succesful.
         */
        virtual bool componentLoaded(RTT::TaskContext* c);

        /**
         * Hook function for subclasses. Allows a subclass
         * to take notice of a component being deleted.
         * @param c a valid TaskContext object.
         */
        virtual void componentUnloaded(RTT::TaskContext* c);

        /**
         * Converts a dot-separated path to a service to a Service
         * object.
         * @param name a dot-separated path name to a service. The first
         * part of the name must be the component name. For example 'Controller.arm'.
         * @return null if the service could not be found, the service
         * otherwise
         */
        Service::shared_ptr stringToService(std::string const& names);
        /**
         * Converts a dot-separated path to a service to a ServiceRequester
         * object.
         * @param name a dot-separated path name to a service. The first
         * part of the name must be the component name. For example 'Controller.arm'.
         * @return null if the service could not be found, the service
         * otherwise
         */
        ServiceRequester* stringToServiceRequester(std::string const& names);
        /**
         * Converts a dot-separated path to a service to a Port
         * object.
         * @param name a dot-separated path name to a port. The first
         * part of the name must be the component name. Example: "Controller.arm.input".
         * @return null if the port could not be found, the port
         * otherwise
         */
        base::PortInterface* stringToPort(std::string const& names);

        /**
         * Waits for any signal and then returns.
         * @return false if this function could not install a signal handler.
         */
        bool waitForSignal(int signumber);

        /**
         * Waits for SIGINT and then returns.
         * @return false if this function could not install a signal handler.
         */
        bool waitForInterrupt();

    public:
        /**
         * Constructs and configures this component.
         *
         * The constructor looks for the site local configuration XML
         * file ('\a name + "-site.cpf"') and if found, kickStart()'s
         * it. You need to set AutoConf to true in order to force a
         * call to configureHook(). In case this file is not present
         * in the current working directory, the component is configured
         * and is thus constructed in the Stopped state. Using a site file
         * does not prevent you from kickstarting or loading other XML files
         * lateron.
         *
         * @param name The name of this component. By default: \a Deployer
         * @param siteFile The site-specific XML file which, if found, will be used
         * for a site-specific kickStart. If left empty, the value becomes
         * by default: \a name + \a "-site.cpf"
         * @see kickStart
         * @see configureHook
         */
        DeploymentComponent(std::string name = "Deployer", std::string siteFile = "");

        /**
         * Cleans up all configuration related information.
         * If the property 'AutoUnload' is set to true, it will
         * also call kickOutAll(), otherwise, the loaded
         * components are left as-is.
         */
        ~DeploymentComponent();

        RTT::TaskContext* myGetPeer(std::string name) {return comps[ name ].instance; }

        /**
         * Make two components peers in both directions, such that both can
         * use each other's services.
         *
         * @param one The component that must 'see' \a other.
         * @param other The component that must 'see' \a one.
         *
         * @return true if both components are peers of this deployment component and
         * could be connected.
         */
        bool connectPeers(const std::string& one, const std::string& other);

        using TaskContext::connectPorts;
        /**
         * Establish a data flow connection between two tasks. The direction
         * of the connection is determined by the read/write port types.
         *
         * @note Using this function is not advised, since it relies on equal
         * port names on both components. Use the alternative form of
         * connectPorts() which specify component/service and
         * port name.
         *
         * @deprecated by connect()
         *
         * @param one The first component
         * @param other The second component
         *
         * @return true if both tasks are peers of this component and
         * data ports could be connected.
         */
        bool connectPorts(const std::string& one, const std::string& other);

        /**
         * Connect two named ports of components. The direction
         * of the connection is determined by the read/write port types.
         *
         * @param one Name of the first component or a dot-separated path to its service
         * @param one_port Name of the port of the first component to connect to
         * \a other_port
         * @param other Name of the second component or a dot-separated path to its service
         * @param other_port Name of the port of the second component to connect
         * to \a one_port
         *
         * @deprecated by connect()
         *
         * @return true if the ports are present and could be connected, false otherwise.
         */
        bool connectPorts(const std::string& one, const std::string& one_port,
                          const std::string& other, const std::string& other_port);

        /**
         * Connect two named ports of components. The direction
         * of the connection is determined by the read/write port types.
         *
         * @param one A dot-separated path to a port in a service
         * @param other A dot-separated path to a port in a service
         * @param policy The connection policy of the new connetion.
         *
         * @return true if the ports are present and could be connected, false otherwise.
         */
        bool connect(const std::string& one, const std::string& other, ConnPolicy policy);

        /**
         * Creates a stream from a given port of a component.
         * @param port The dot-separated path to an input or output port of \a component or service.
         * @param policy The connection policy that instructs how to set up the stream.
         * The policy.transport field is mandatory, the policy.name_id field is highly recommended.
         * @return true if the stream could be setup.
         */
        bool stream(const std::string& port, ConnPolicy policy);

        /**
         * @deprecated by stream()
         */
        bool createStream(const std::string& component, const std::string& port, ConnPolicy policy);

        using TaskContext::connectServices;
        /**
         * Connects the required services of one component to the provided services
         * of another and vice versa.
         * @param one The name of a provided service or a dot-separated path to a Service
         * @param one The name of a required service or a dot-separated path to a ServiceRequester
         * @note One of the parameters must be a Service and the other a ServiceRequester,
         * or vice versa.
         */
        bool connectServices(const std::string& one, const std::string& other);

        /**
         * Connects a required operation to a provided operation.
         * @param required The dot-separated name of a required operation
         * @param provided The dot-separated name of a provided operation
         * @return true if required is connected to provided
         */
        bool connectOperations(const std::string& required, const std::string& provided);

        /**
         * Make one component a peer of the other, in one direction, such
         * that one can use the services of the other.
         *
         * @param from The component that must 'see' \a target and use its services.
         * @param target The component that is 'seen' and used by \a from.
         *
         * @return true if both components are peers of this deployment component and
         * target became a peer of from.
         */
        bool addPeer(const std::string& from, const std::string& target);

        /**
         * Make one component a peer of the other, in one direction, with an alternative name, such
         * that one can use the services of the other and knows it under the name of the alias.
         *
         * @param from The component that must 'see' \a target and use its services.
         * @param target The component that is 'seen' and used by \a from.
         * @param alias The name of the target as it will be seen by \a from.
         *
         * @return true if both components are peers of this deployment component and
         * target became a peer of from.
         */
        bool aliasPeer(const std::string& from, const std::string& target, const std::string& alias);

        using RTT::TaskContext::addPeer;
        using RTT::TaskContext::connectPeers;

        /**
         * Import a component package or directory.
         * The import statement searches through the component paths
         * set with path() or set using the RTT_COMPONENT_PATH
         * environment variable.
         *
         * @param package A (ros) package or directory name. All components,
         * plugins and typekits in package will be loaded and become
         * available to the application. As a special case, you may
         * specify a path to a library directly, which will be loaded when
         * found.
         * @return true if the package could be found and loaded,
         * false if no such package was found in the search path.
         */
        bool import(const std::string& package);

        /**
         * Add an additional path to search for component packages.
         *
         * @param a colon or semi-colon separated list of
         * directories to search for. Typically, paths have
         * the form prefix1/lib/orocos:prefix2/lib/orocos etc.
         */
        void path(const std::string& path);

        /**
         * Use this command to load a plugin or component library into the memory of the
         * current process. This is a low-level function which you should
         * only use if you could not use import().
         *
         * @param name an absolute or relative path to a loadable library.
         *
         * @return True if it could be loaded, false otherwise.
         */
        bool loadLibrary(const std::string& name);

        /**
         * Use this command to \b reload a \b component library into the memory of the
         * current process. This is a low-level function which you should
         * only use for testing/development
         *
         * @param filepath an \b absolute path to a loaded library.
         *
         * @return True if it could be reloaded, false otherwise.
         */
        bool reloadLibrary(const std::string& filepath);

        /**
         * Load a new component in the current process. It wil appear
         * as a peer with name \a name of this component.
         *
         * @param name Name the new component will receive.
         * @param type The type of the component. This is usually a library (.dll or .so)
         * name.
         *
         * @return True if the component could be created, false if \a name
         * is already in use or \a type was not an Orocos library.
         */
        bool loadComponent(const std::string& name, const std::string& type);

        /**
         * Loads a service in the given component.
         * If a service with the name of \a service is already loaded,
         * does nothing and returns true.
         *
         * @note If the loaded service uses another name to expose itself
         * or does not register itself with an RTT::Service object, this function will
         * load the service anyway in case it has been loaded before.
         *
         * @param component A peer of this component.
         * @param service A service discovered by the PluginLoader which will be
         * loaded into \a component.
         * @return false if component is not a peer or service is not known. true otherwise.
         */
        bool loadService(const std::string& component, const std::string& service);

        /**
         * Unload a loaded component from the current process. It may not
         * be running.
         *
         * @param name The name of a component loaded with loadComponent().
         *
         * @return true if \a name was not running and could be unloaded.
         */
        bool unloadComponent(const std::string& name);

        /**
         * This function prints out the component types this DeploymentComponent
         * can create.
         * @see loadComponent()
         */
        void displayComponentTypes() const;

        /**
         * This function returns the component types this DeploymentComponent
         * can create in a comma separated list.
         * @see loadComponent()
         */
        std::vector<std::string> getComponentTypes() const;

        /**
         * (Re-)set the activity of a component with a periodic activity.
         *
         * @param comp_name The name of the component to change.
         * @param period    The period of the activity.
         * @param priority  The scheduler priority (OS dependent).
         * @param scheduler The scheduler type \a ORO_SCHED_RT or \a ORO_SCHED_OTHER.
         *
         * @return false if one of the parameters does not match or if the
         * component is running.
         */
        bool setPeriodicActivity(const std::string& comp_name,
                                 double period, int priority,
                                 int scheduler);

        /**
         * (Re-)set the activity of a component with an activity.
         *
         * @param comp_name The name of the component to change.
         * @param period    The period of the activity (or 0.0 if non periodic).
         * @param priority  The scheduler priority (OS dependent).
         * @param scheduler The scheduler type \a ORO_SCHED_RT or \a ORO_SCHED_OTHER.
         *
         * @return false if one of the parameters does not match or if the
         * component is running.
         */
        bool setActivity(const std::string& comp_name,
                         double period, int priority,
                         int scheduler);

        /**
         * (Re-)set the activity of a component with a FileDescriptor activity.
         *
         * @param comp_name The name of the component to change.
         * @param timeout   The timeout of the activity (or 0.0 if no timeout).
         * @param priority  The scheduler priority (OS dependent).
         * @param scheduler The scheduler type \a ORO_SCHED_RT or \a ORO_SCHED_OTHER.
         *
         * @return false if one of the parameters does not match or if the
         * component is running.
         */
        bool setFileDescriptorActivity(const std::string& comp_name,
                         double timeout, int priority,
                         int scheduler);
						 
        /**
         * (Re-)set the activity of a component and run it on a given CPU.
         *
         * @param comp_name The name of the component to change.
         * @param period    The period of the activity (or 0.0 if non periodic).
         * @param priority  The scheduler priority (OS dependent).
         * @param scheduler The scheduler type \a ORO_SCHED_RT or \a ORO_SCHED_OTHER.
	 * @param cpu_nr    The CPU to run the thread on. Numbering starts from zero.
         *
         * @return false if one of the parameters does not match or if the
         * component is running.
         */
        bool setActivityOnCPU(const std::string& comp_name,
                         double period, int priority,
			      int scheduler, unsigned int cpu_nr);

        /**
         * (Re-)set the activity of a component with a (threadless, reactive) sequential activity.
         *
         * @param comp_name The name of the component to change.
         *
         * @return false if one of the parameters does not match or if the
         * component is running.
         */
        bool setSequentialActivity(const std::string& comp_name);

        /**
         * (Re-)set the activity of a component with a (stand alone) slave activity.
         *
         * @param comp_name The name of the component to change.
         * @param period    The period of the activity.
         *
         * @return false if one of the parameters does not match or if the
         * component is running.
         */
        bool setSlaveActivity(const std::string& comp_name,
                              double period);

        /**
         * (Re-)set the activity of a component with a slave activity with master.
         *
         * @param comp_name The name of the component to change.
         * @param master_name The name of the master component.
         *
         * @return false if one of the components is not found or \a comp_name is running.
         */
        bool setMasterSlaveActivity(const std::string& comp_name,
                                    const std::string& master_name);

        /**
         * (Re-)set the activity of a component.
         *
         * CPU affinity defaults to all available CPUs
         *
         * @param comp_name The name of the component to change.
         * @param act_type  The RTT::Activity type: 'Activity', 'PeriodicActivity', 'SequentialActivity' or 'SlaveActivity'.
         * @param priority  The scheduler priority (OS dependent).
         * @param period    The period of the activity.
         * @param scheduler The scheduler type \a ORO_SCHED_RT or \a ORO_SCHED_OTHER.
         * @param master_name The name of the master component in case of a extras::SlaveActivity with a master.
         *
         * @return false if one of the parameters does not match or if the
         * component is running.
         */
        bool setNamedActivity(const std::string& comp_name,
                         const std::string& act_type,
                         double period, int priority,
                         int scheduler, const std::string& master_name = "");

        /**
         * (Re-)set the activity of a component.
         *
         * @param comp_name The name of the component to change.
         * @param act_type  The RTT::Activity type: 'Activity', 'PeriodicActivity', 'SequentialActivity' or 'SlaveActivity'.
         * @param priority  The scheduler priority (OS dependent).
         * @param period    The period of the activity.
         * @param scheduler The scheduler type \a ORO_SCHED_RT or \a ORO_SCHED_OTHER.
         * @param cpu_affinity The prefered cpu to run on (a mask)
         * @param master_name The name of the master component in case of a extras::SlaveActivity with a master.
         *
         * @return false if one of the parameters does not match or if the
         * component is running.
         */
        bool setNamedActivity(const std::string& comp_name,
                         const std::string& act_type,
                         double period, int priority,
                         int scheduler, unsigned cpu_affinity,
                         const std::string& master_name = "");

        /**
         * Load a (partial) application XML configuration from disk. The
         * necessary components are located or loaded, but no
         * component configuration is yet applied. One can load
         * multiple configurations and call configureComponents() once
         * to apply all settings. In case of duplicate information is
         * the latest loaded configuration option used. The components
         * are loaded into the next group number, and the next group
         * number is incremented.
         *
         * @see configureComponents to configure the components with
         * the loaded configuration and startComponents to start them.
         * @param config_file A file on local disk containing the XML configuration.
         * @return true if the configuration could be read and was valid.
         */
        bool loadComponents(const std::string& config_file);
        /** 
         * Load a (partial) application XML configuration from disk into a 
         * specific group.
         *
         * If the group already exists, then adds to that group. Does not
         * affect existing members of the group.
         *
         * @see loadComponents for general details
         * @param config_file A file on local disk containing the XML configuration.
         * @param group The group number to load into
         * @pre 0 <= group
         * @return true if the configuration could be read and was valid.
         */
        bool loadComponentsInGroup(const std::string& config_file,
                                   const int group);

        /**
         * Configure the components with loaded configuration(s). This
         * function connects components and data ports, reads
         * properties for the components, attaches activities and
         * loads program and state machine scripts.  If a component
         * XML entry has the AutoConf element, configure() will be
         * called upon this component as well.  If the configuration
         * fails halfway, the system is configured as complete as
         * possible. You can try to reconfigure by loading a new
         * configuration (using loadConfiguration ) and call
         * configureComponents again to resolve remaining issues.
         *
         * This function tries to apply the configuration with a best effort.
         * For example, if a program must be loaded in the component, and a program
         * with that same name is already present, the present one is unloaded and the
         * new one is attempted to be loaded. If that fails, the configuration process
         * leaves the scripts as-is and proceeds with further configuration steps of
         * the same component and other components.
         *
         * The order of configuration depends on the order of components during
         * loadConfiguration. The first encountered component is configured first.
         * If additional loadConfiguration operations refer to the same component,
         * the configuration order is not changed.
         *
         * @return true if all components could be succesfully configured.
         */
        bool configureComponents();

        /**
         * Configure the components in group \a group.
         *
         * @see configureComponents()
         *
         * @return true if all the group's components could be succesfully configured.
         */
        bool configureComponentsGroup(const int group);

        /**
         * Start all components in the current configuration which have AutoStart
         * set to true.
         * @return true if all components could be succesfully started.
         */
        bool startComponents();
        /**
         * Start all components in group \a group which have AutoStart
         * set to true.
         * @return true if all the group's components could be succesfully started.
         */
        bool startComponentsGroup(const int group);

        /**
         * Clear all loaded configuration options.
         * This does not alter any component.
         */
        void clearConfiguration();

        /**
         * Stop all loaded and running components.
         */
        bool stopComponents();
        /**
         * Stop all loaded and running components in group \a group.
         *
         * @param group The group number to stop
         */
        bool stopComponentsGroup(const int group);

        /**
         * Cleanup all loaded and not running components.
         */
        bool cleanupComponents();
        /**
         * Cleanup all loaded and not running components.
         *
         * @param group The group number to cleanup
         */
        bool cleanupComponentsGroup(const int group);        

        /**
         * Unload all loaded and not running components.
         */
        bool unloadComponents();
        /**
         * Unload all loaded and not running components in group \a group
         *
         * @param group The group number to unload
         */
        bool unloadComponentsGroup(const int group);

        /**
         * This function runs loadComponents, configureComponents and startComponents
         * in a row, given no failures occur along the way.
         */
        bool kickStart(const std::string& file_name);

        /**
         * Stop, cleanup and unload a single component which were loaded by this component.
         * @param comp_name name of the component.
         * @return true if successfully stopped, cleaned and unloaded
         */
        bool kickOutComponent(const std::string& comp_name);

        /**
         * Identical to \a kickOutAll, but it reads the name of the Components to kickOut from an XML file.
         * @param config_file name of an XML file (probably the same used by loadComponents() or kickStart() ).
         */
        void kickOut(const std::string& config_file);

        /**
         * Stop, cleanup and unload all components loaded by the DeploymentComponent.
         *
         * @post 0 == nextGroup
         */
        bool kickOutAll();
        /**
         *
         * Stop, cleanup and unload all components in group \a group.
         */
        bool kickOutGroup(const int group);

        /**
         * Scripting-alternative to kickStart: runs this script in the Orocos
         * scripting service.
         */
        bool runScript(const std::string& file_name);

        using base::TaskCore::configure;

        /**
         * Configure a component by loading the property file 'name.cpf' for component with
         * name \a name.
         * @param name The name of the component to configure.
         * The file used will be 'name.cpf'.
         * @return true if the component is a peer of this component and the file could be
         * read.
         */
        bool configure(const std::string& name);

        /**
         * Configure a component by loading a property file.
         *
         * @param name The name of the component to configure
         * @param filename The filename where the configuration is in.
         *
         * @return true if the component is a peer of this component and the file could be
         * read.
         */
        bool configureFromFile(const std::string& name, const std::string& filename);

        /**
         * Load a (partial) application XML configuration from disk. The
         * necessary components are located or loaded, but no
         * component configuration is yet applied. One can load
         * multiple configurations and call configureComponents() once
         * to apply all settings. In case of duplicate information is
         * the latest loaded configuration option used.
         *
         * @deprecated by loadComponents.
         * @see configureComponents to configure the components with
         * the loaded configuration.
         * @param config_file A file on local disk containing the XML configuration.
         * @return true if the configuration could be read and was valid.
         */
        bool loadConfiguration(const std::string& config_file);

        /**
         * Identical to \a loadConfiguration, but reads the XML from a string
         * instead of a file.
         * @param config_text A string containing the XML configuration.
         * @return true if the configuration string could be read and was valid.
         */
        bool loadConfigurationString(const std::string& config_text);

        /**
         * Returns the factory singleton which creates all types of components
         * for the DeploymentComponent.
         */
        const RTT::FactoryMap& getFactories() const;

        /**
         * Stop a single loaded and running component.
         * @param instance instance pointer of the component.
         * @return true if successfully stopped.
         */
        bool stopComponent(RTT::TaskContext *instance);

        /**
         * Stop a single loaded and running components.
         * @param comp_name name of the component.
         * @return true if successfully stopped
         */
        bool stopComponent(const std::string& comp_name)
        {
            return this->stopComponent(  this->getPeer(comp_name) );
        }

        /**
         * Cleanup a single loaded and not running component.
         * @param instance instance pointer of the component.
         * @return true if successfully cleaned up
         */
        bool cleanupComponent(RTT::TaskContext *instance);

        /**
         * Cleanup a single loaded and not running component.
         * @param comp_name name of the component.
         * @return true if successfully cleaned up
         */
        bool cleanupComponent(const std::string& comp_name)
        {
            return this->cleanupComponent( this->getPeer(comp_name) );
        }

		/**
		 * Clean up and shutdown the entire deployment
		 * If an operation named "shutdownDeployment" is found in a peer
         * component named "Application", then that operation is called
         * otherwise nothing occurs.
		 */
		void shutdownDeployment();

    };


}
#endif
