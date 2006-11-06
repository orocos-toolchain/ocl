#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Logger.hpp>
#include <rtt/PropertyLoader.hpp>
#include <pkgconf/corelib_properties_marshalling.h>
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

        struct ConnectionData {
            typedef std::vector<RTT::PortInterface*> Ports;
            Ports ports;
        };

        typedef std::map<std::string, ConnectionData> ConMap;
        ConMap conmap;
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
         * Load a configuration from disk. The 'ConfigFile' property is used to
         * locate the file. This does not apply the configuration yet on the
         * components.
         * @see configurePeers to configure the peer components with the loaded
         * configuration.
         * 
         * @return true if the configuration could be read and was valid.
         */
        bool loadConfiguration();

        /** 
         * Configure the components with a loaded configuration.
         * 
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
