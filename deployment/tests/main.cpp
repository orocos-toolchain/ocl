
#include "deployment/DeploymentComponent.hpp"
#include "taskbrowser/TaskBrowser.hpp"
#include <rtt/DataPort.hpp>
#include <rtt/os/main.h>
#include <rtt/RTT.hpp>

using namespace Orocos;

class MyTask
    : public TaskContext
{
public:
    ReadDataPort<double> p1;
    WriteDataPort<double> p2;
    DataPort<double> p3;

    MyTask(std::string n)
        : TaskContext(n),
          p1("p1"),
          p2("p2", 0.0),
          p3("p3", 0.0)
    {
        this->ports()->addPort( &p1, "");
        this->ports()->addPort( &p2, "");
        this->ports()->addPort( &p3, "");
    }
};

int ORO_main(int, char**)
{
    DeploymentComponent dc;
    MyTask t1("t1");
    MyTask t2("t2");

    dc.addPeer( &t1 );
    dc.addPeer( &t2 );

    TaskBrowser tb(&dc);

    tb.loop();

    return 0;
}
