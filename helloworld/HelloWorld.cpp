
/**
 * @file HelloWorld.cpp
 * This file demonstrate each Orocos primitive with
 * a 'hello world' example.
 */

#include <rtt/os/main.h>

#include <rtt/TaskContext.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Port.hpp>
#include <rtt/Activity.hpp>

#include <ocl/OCL.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::detail; // workaround in 2.0 transition phase.
using namespace Orocos;

namespace OCL
{

    /**
     * Every component inherits from the 'RTT::TaskContext' class.  This base
     * class allow a user to add a primitive to the interface and contain
     * an RTT::ExecutionEngine which executes application code.
     */
    class HelloWorld
        : public RTT::TaskContext
    {
    protected:
        /**
         * @name Name-Value parameters
         * @{
         */
        /**
         * Properties take a name, a value and a description
         * and are suitable for XML.
         */
        std::string property;

        /**
         * Attribute that you can toggle to influence what is printed
         * in updateHook()
         */
        bool flag;
        /**
         * Attributes are aliased to class variables.
         */
        std::string attribute;
        /**
         * Constants are aliased, but can only be changed
         * from the component itself.
         */
        std::string constant;
        /** @} */

        /**
         * @name Input-Output ports
         * @{
         */
        /**
         * We publish our data through this RTT::OutputPort
         *
         */
        RTT::OutputPort<std::string> outport;
        /**
         * This RTT::InputPort buffers incoming data.
         */
        RTT::InputPort<std::string> bufferport;
        /** @} */

        /**
         * An operation we want to add to our interface.
         */
        std::string mymethod() {
            return "Hello World";
        }

        /**
         * This one is executed in our own thread.
         */
        bool sayWorld( const std::string& word) {
            cout <<"Saying Hello '"<<word<<"' in own thread." <<endl;
            if (word == "World")
                return true;
            return false;
        }

        void updateHook() {
        	if (flag) {
        		cout <<"flag: " << flag <<endl;
        		cout <<"the_property: "<< property <<endl;
        		cout <<"the_attribute: "<< attribute <<endl;
        		cout <<"the_constant: "<< constant <<endl;
        		cout <<"Setting 'flag' back to false."<<endl;
        		flag = false;
        	}
        }
    public:
        /**
         * This example sets the interface up in the Constructor
         * of the component.
         */
        HelloWorld(std::string name)
            : RTT::TaskContext(name),
              // Name, description, value
              property("Hello Property"), flag(false),
              attribute("Hello Attribute"),
              constant("Hello Constant"),
              // Name, initial value
              outport("the_results",true),
              // Name, policy
              bufferport("the_buffer_port",ConnPolicy::buffer(13,ConnPolicy::LOCK_FREE,true) )
        {
            // New activity with period 0.1s and priority 0.
            this->setActivity( new Activity(0, 0.1) );

            // Set log level more verbose than default,
            // such that we can see output :
            if ( log().getLogLevel() < RTT::Logger::Info ) {
                log().setLogLevel( RTT::Logger::Info );
                log(Info) << "HelloWorld manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<endlog();
            }

            // Now add member variables to the interface:
            this->properties()->addProperty("the_property", property).doc("A friendly property.");

            this->addAttribute("flag", flag);
            this->addAttribute("the_attribute", attribute);
            this->addConstant("the_constant", constant);

            this->ports()->addPort( outport );
            this->ports()->addPort( bufferport );

            this->addOperation( "the_method", &HelloWorld::mymethod, this, ClientThread ).doc("'the_method' Description");

            this->addOperation( "the_command", &HelloWorld::sayWorld, this, OwnThread).doc("'the_command' Description").arg("the_arg", "Use 'World' as argument to make the command succeed.");

            log(Info) << "**** Starting the 'Hello' component ****" <<endlog();
            // Start the component's activity:
            this->start();
        }
    };
}


// This define allows to compile the hello world component as a library
// liborocos-helloworld.so or as a program (helloworld). Your component
// should only be compiled as a library.
#ifndef OCL_COMPONENT_ONLY

int ORO_main(int argc, char** argv)
{
    RTT::Logger::In in("main()");

    // Set log level more verbose than default,
    // such that we can see output :
    if ( log().getLogLevel() < RTT::Logger::Info ) {
        log().setLogLevel( RTT::Logger::Info );
        log(Info) << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<endlog();
    }

    log(Info) << "**** Creating the 'Hello' component ****" <<endlog();
    // Create the task:
    HelloWorld hello("Hello");

    log(Info) << "**** Using the 'Hello' component    ****" <<endlog();

    // Do some 'client' calls :
    log(Info) << "**** Reading a RTT::Property:            ****" <<endlog();
    RTT::Property<std::string> p = hello.properties()->getProperty("the_property");
    assert( p.ready() );
    log(Info) << "     "<<p.getName() << " = " << p.value() <<endlog();
#if 0
    log(Info) << "**** Sending a RTT::OperationCaller:             ****" <<endlog();
    RTT::OperationCaller<bool(std::string)> c = hello.getOperation<bool(std::string)>("the_command");
    assert( c.ready() );
    log(Info) << "     Sending RTT::OperationCaller : " << c.send("World")<<endlog();

    log(Info) << "**** Calling a RTT::OperationCaller:              ****" <<endlog();
    RTT::OperationCaller<std::string(void)> m = hello.getOperation<std::string(void)>("the_method");
    assert( m.ready() );
    log(Info) << "     Calling RTT::OperationCaller : " << m() << endlog();
#endif
    log(Info) << "**** Starting the TaskBrowser       ****" <<endlog();
    // Switch to user-interactive mode.
    TaskBrowser browser( &hello );

    // Accept user commands from console.
    browser.loop();

    return 0;
}

#else

#include "ocl/Component.hpp"
ORO_CREATE_COMPONENT( OCL::HelloWorld )

#endif
