
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
        this->properties()->addProperty(&limits);
        this->ports()->addPort( &p1 );
        this->ports()->addPort( &p2 );
        this->ports()->addPort( &p3 );
        this->addAttribute( &a );
        limits.set()[0] = 10;
        limits.set()[1] = 20;
    }
};

int ORO_main(int, char**)
{
    MyTask t1("ComponentA");
    MyTask t2("ComponentB");

    {
        DeploymentComponent dc;
        dc.addPeer( &t1 );
        dc.addPeer( &t2 );
        dc.kickStart("deployment.cpf");
        TaskBrowser tb(&dc);

        tb.loop();
    }
    return 0;
}
