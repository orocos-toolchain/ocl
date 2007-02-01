#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Logger.hpp>
#include <rtt/PropertyLoader.hpp>
#include <rtt/marsh/CPFDemarshaller.hpp>

#include <ocl/OCL.hpp>
namespace OCL
{

    /**
     * A Component for deploying (configuring) other components in an
     * application. It allows to create connections between components and
     * load the properties for components,
     * such that connection of ports with different names can take place.
     */
    class DeploymentComponent
        : public RTT::TaskContext
    {
    protected:
        RTT::PropertyBag root;
        // STRUCTURE:
        //         Property<PropertyBag> component;
        //         //         Property<std::string> property-source;
        
        //         Property<PropertyBag> ports;
        //         //         Property<std::string> portname; // value = connection_name
        
        //         Property<std::string> peername;

        RTT::Property<std::string> configurationfile;
        RTT::Property<bool> autoConnect;
        RTT::Attribute<bool> validConfig;

        /**
         * Each loaded component is stored in a struct like this.
         */
        struct ComponentData {
            /**
             * The created component instance.
             */
            RTT::TaskContext* instance;
            /**
             * The 'main' class type
             */
            std::string type;
            /**
             * The property file (if any).
             */
            std::string properties;
        };

        struct ConnectionData {
            typedef std::vector<RTT::PortInterface*> Ports;
            Ports ports;
        };

        typedef std::map<std::string, ConnectionData> ConMap;
        ConMap conmap;

        typedef std::vector<ComponentData> CompList;
        CompList comps;
        
    public:
        DeploymentComponent(std::string name = "Configurator");
        
        ~DeploymentComponent();

        /** 
         * Establish a bidirectional connection between two tasks.
         * 
         * @param one The first task to connect
         * @param other The second task to connect
         * 
         * @return true if both tasks are peers of this component and
         * could be connected.
         */
        bool connectPeers(const std::string& one, const std::string& other);

        /** 
         * Establish a uni directional connection form one task to another
         * 
         * @param from The component where the connection starts.
         * @param to The component where the connection ends.
         * 
         * @return true if both tasks are peers of this component and
         * a connection could be created.
         */
        bool addPeer(const std::string& from, const std::string& to);

        using TaskContext::addPeer;
        using TaskContext::connectPeers;

        /** 
         * Load a new component in the current process.
         * 
         * @param name 
         * @param type 
         * 
         * @return 
         */
        bool loadComponent(const std::string& name, const std::string& type);

        /** 
         * Unload a loaded component from the current process.
         * 
         * @param name 
         * 
         * @return 
         */
        bool unloadComponent(const std::string& name);

        /**
         * (Re-)set the activity of a component.
         * 
         * @param comp_name The name of the component to change.
         * @param act_type  The Activity type: 'PeriodicActivity', 'NonPeriodicActivity' or 'SlaveActivity'.
         * @param period    The period of the activity. Must be \a 0.0 in case of NonPeriodicActivity.
         * @param priority  The scheduler priority (OS dependent).
         * @param scheduler The scheduler type \a ORO_SCHED_RT or \a ORO_SCHED_OTHER.
         * 
         * @return false if one of the parameters does not match or if the
         * component is running.
         */
        bool setActivity(const std::string& comp_name, 
                         const std::string& act_type, 
                         double period, int priority,
                         const std::string& scheduler);

        /** 
         * Load a configuration from disk. The 'ConfigFile' property is used to
         * locate the file. This does not apply the configuration yet on the
         * components.
         * @see configureComponents to configure the peer components with the loaded
         * configuration.
         * 
         * @return true if the configuration could be read and was valid.
         */
        bool loadConfiguration();

        /** 
         * Configure the components with a loaded configuration.
         * 
         * @return true if all components could be succesfully configured.
         */
        bool configureComponents();

        /**
         * Configure a component by loading the property file 'name.cpf' for component with
         * name \a name.
         * @param name The name of the component to configure.
         * The file used will be 'name.cpf'.
         * @return true if the component is known and the file could be
         * read.
         */
        bool configure(std::string name);

        /** 
         * Configure a component by loading a property file.
         * 
         * @param name The name of the component to configure
         * @param filename The filename where the configuration is in.
         * 
         * @return true if the component is known and the file could be
         * read.
         */
        bool configureFromFile(std::string name, std::string filename);
    };


}
