
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
#include <rtt/Method.hpp>
#include <rtt/Command.hpp>
#include <rtt/Event.hpp>
#include <rtt/Ports.hpp>
#include <rtt/PeriodicActivity.hpp>

#include <ocl/OCL.hpp>

using namespace std;
using namespace RTT;
using namespace Orocos;

namespace OCL
{

    /**
     * Every component inherits from the 'TaskContext' class.  This base
     * class allow a user to add a primitive to the interface and contain
     * an ExecutionEngine which executes application code.
     */
    class HelloWorld
        : public TaskContext
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
        Property<std::string> property;
        /**
         * Attributes take a name and contain changing values.
         */
        Attribute<std::string> attribute;
        /**
         * Constants take a name and contain a constant value.
         */
        Constant<std::string> constant;
        /** @} */

        /**
         * @name Input-Output ports
         * @{
         */
        /**
         * DataPorts share data among readers and writers.
         * A reader always reads the most recent data.
         */
        DataPort<std::string> dataport;
        /**
         * BufferPorts buffer data among readers and writers.
         * A reader reads the data in a FIFO way.
         */
        BufferPort<std::string> bufferport;
        /** @} */

        /**
         * @name Method
         * @{
         */
        /**
         * Methods take a number of arguments and
         * return a value. The are executed in the
         * thread of the caller.
         */
        Method<std::string(void)> method;

        /**
         * The method function is executed by
         * the method object:
         */
        std::string mymethod() {
            return "Hello World";
        }
        /** @} */

        /**
         * @name Command
         * @{
         */
        /**
         * Commands take a number of arguments and
         * return true or false. They are asynchronous
         * and executed in the thread of the receiver.
         */
        Command<bool(std::string)> command;

        /**
         * The command function executed by the receiver.
         */
        bool mycommand(std::string arg) {
            log() << "Hello Command: "<< arg << endlog();
            if ( arg == "World" )
                return true;
            else
                return false;
        }

        /**
         * The completion condition checked by the sender.
         */
        bool mycomplete(std::string arg) {
            log() << "Checking Command: "<< arg <<endlog();
            return true;
        }
        /** @} */

        /**
         * @name Event
         * @{
         */
        /**
         * The event takes a payload which is distributed
         * to anonymous receivers. Distribution can happen
         * synchronous and asynchronous.
         */
        Event<void(std::string)> event;

        /**
         * Stores the connection between 'event' and 'mycallback'.
         */
        Handle h;

        /**
         * An event callback (or subscriber) function.
         */
        void mycallback( std::string data )
        {
            log() << "Receiving Event: " << data << endlog();
        }
        /** @} */

        PeriodicActivity act;

    public:
        /**
         * This example sets the interface up in the Constructor
         * of the component.
         */
        HelloWorld(std::string name)
            : TaskContext(name),
              // Name, description, value
              property("the_property", "the_property Description", "Hello World"),
              // Name, value
              attribute("the_attribute", "Hello World"),
              // Name, value
              constant("the_constant", "Hello World"),
              // Name, initial value
              dataport("the_data_port","World"),
              // Name, buffer size, initial value
              bufferport("the_buffer_port",13, "World"),
              // Name, function pointer, object
              method("the_method", &HelloWorld::mymethod, this),
              // Name, command function pointer, completion condition function pointer, object
              command("the_command", &HelloWorld::mycommand, &HelloWorld::mycomplete, this),
              // Name
              event("the_event"),

              // Create the activity which runs the task's engine:
              // 0: Priority
              // 0.01: Period (100Hz)
              // engine(): is being executed.
              act(0, 0.01, this->engine() )
        {
            // Set log level more verbose than default,
            // such that we can see output :
            if ( log().getLogLevel() < Logger::Info ) {
                log().setLogLevel( Logger::Info );
                log(Info) << "HelloWorld manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<endlog();
            }

            // Check if all initialisation was ok:
            assert( property.ready() );
            assert( attribute.ready() );
            assert( constant.ready() );
            assert( method.ready() );
            assert( command.ready() );
            assert( event.ready() );

            // Now add it to the interface:
            this->properties()->addProperty(&property);

            this->attributes()->addAttribute(&attribute);
            this->attributes()->addConstant(&constant);

            this->ports()->addPort(&dataport);
            this->ports()->addPort(&bufferport);

            this->methods()->addMethod(&method, "'the_method' Description");
        
            this->commands()->addCommand(&command, "'the_command' Description",
                                         "the_arg", "Use 'World' as argument to make the command succeed.");

            this->events()->addEvent(&event, "'the_event' Description",
                                     "the_data", "The data of this event.");

            // Adding an asynchronous callback:
            h = this->events()->setupConnection("the_event").callback(this, &HelloWorld::mycallback, this->engine()->events() ).handle();
            h.connect();

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
    Logger::In in("main()");

    // Set log level more verbose than default,
    // such that we can see output :
    if ( log().getLogLevel() < Logger::Info ) {
        log().setLogLevel( Logger::Info );
        log(Info) << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<endlog();
    }

    log(Info) << "**** Creating the 'Hello' component ****" <<endlog();
    // Create the task:
    HelloWorld hello("Hello");

    log(Info) << "**** Using the 'Hello' component    ****" <<endlog();

    // Do some 'client' calls :
    log(Info) << "**** Reading a Property:            ****" <<endlog();
    Property<std::string> p = hello.properties()->getProperty<std::string>("the_property");
    assert( p.ready() );
    log(Info) << "     "<<p.getName() << " = " << p.value() <<endlog();

    log(Info) << "**** Sending a Command:             ****" <<endlog();
    Command<bool(std::string)> c = hello.commands()->getCommand<bool(std::string)>("the_command");
    assert( c.ready() );
    log(Info) << "     Sending Command : " << c("World")<<endlog();

    log(Info) << "**** Calling a Method:              ****" <<endlog();
    Method<std::string(void)> m = hello.methods()->getMethod<std::string(void)>("the_method");
    assert( m.ready() );
    log(Info) << "     Calling Method : " << m() << endlog();

    log(Info) << "**** Emitting an Event:             ****" <<endlog();
    Event<void(std::string)> e = hello.events()->getEvent<void(std::string)>("the_event");
    assert( e.ready() );

    e("Hello World");

    log(Info) << "**** Starting the TaskBrowser       ****" <<endlog();
    // Switch to user-interactive mode.
    TaskBrowser browser( &hello );

    // Accept user commands from console.
    browser.loop();

    return 0;
}

#else

#include "ocl/ComponentLoader.hpp"
ORO_CREATE_COMPONENT( OCL::HelloWorld )

#endif
