
#include "deployment/DeploymentComponent.hpp"
#include "taskbrowser/TaskBrowser.hpp"
#include <rtt/InputPort.hpp>
#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <vector>

using namespace Orocos;

class MyTask
    : public RTT::TaskContext
{
public:
    Property<std::vector<double> > limits;
    InputPort<double> p1;
    OutputPort<double> p2;
    OutputPort<double> p3;
    Attribute<double> a;

    MyTask(std::string n)
        : RTT::TaskContext(n),
          limits("limits","desc", std::vector<double>(2) ),
          p1("p1"),
          p2("p2"),
          p3("p3"),
          a("a")
    {
        this->properties()->addProperty( limits);
        this->ports()->addPort( p1 );
        this->ports()->addPort( p2 );
        this->ports()->addPort( p3 );
        this->addAttribute( a );
        limits.set()[0] = 10;
        limits.set()[1] = 20;
    }
};

// creates ports dynamically (ie in configureHook() )
class MyDynamicTask
: public RTT::TaskContext
{
public:
    InputPort<double> d1;
    OutputPort<double> d2;

    MyDynamicTask(std::string n)
        : RTT::TaskContext(n),
        d1("d1"),
        d2("d2")
    {
    }
    virtual bool configureHook()
    {
        this->ports()->addPort( d1 );
        this->ports()->addPort( d2 );
        return true;
    }
};

class HelloProvider
    : public RTT::TaskContext
{
public:
    bool hello() {
        RTT::Logger::In("connectOperations Test");
        log(Info) << "Hello World!" << endlog();
        return true;
    }

    HelloProvider()
    : RTT::TaskContext("HelloProvider")
    {
        this->provides("hello_service")->addOperation("hello", &HelloProvider::hello, this);
    }
};

class HelloRequester
    : public RTT::TaskContext
{
public:
    RTT::OperationCaller<bool()> helloworld;
    HelloRequester()
    : RTT::TaskContext("HelloRequester"),
      helloworld("helloworld")
    {
        this->requires()->addOperationCaller(helloworld);
    }
    void virtual updateHook() {
        helloworld();
    }
};

int ORO_main(int, char**)
{
    RTT::Logger::Instance()->setLogLevel(RTT::Logger::Info);
    int exit_code = 0;

    MyTask t1("ComponentA");
    MyTask t2("ComponentB");
    MyTask t3("ComponentC");
    MyDynamicTask t4("ComponentD");

    HelloProvider p;
    HelloRequester r;

    {
        DeploymentComponent dc;
        dc.addPeer( &t1 );
        dc.addPeer( &t2 );
        dc.addPeer( &t3 );
        dc.addPeer( &t4 );
        dc.addPeer( &p );
        dc.addPeer( &r );
        dc.kickStart("deployment.cpf");

#if defined(RTT_VERSION_GTE)
#if RTT_VERSION_GTE(2,8,99)
        if (RTT::ConnPolicy::Default().size != 99 ||
            RTT::ConnPolicy::Default().buffer_policy != RTT::Shared) {
            log(Fatal) << "Default ConnPolicy not set correctly!" << endlog();
            exit_code = 1;
        }
#endif
#endif

        TaskBrowser tb(&dc);
        tb.loop();
    }
    return exit_code;
}
