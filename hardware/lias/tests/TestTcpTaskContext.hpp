#ifndef TEST_TCP_TASK_CONTEXT_H
#define TEST_TCP_TASK_CONTEXT_H

#include <rtt/RTT.hpp>
#include <rtt/GenericTaskContext.hpp>
//#include <corelib/NonPreemptibleActivity.hpp>
#include <rtt/Attribute.hpp>
//#include <execution/TemplateFactories.hpp>
//#include <execution/TaskBrowser.hpp>
//#include <execution/MethodC.hpp>
//#include <execution/CommandC.hpp>
//#include <execution/EventC.hpp>
//#include <execution/ConnectionC.hpp>
#include <rtt/Ports.hpp>
//#include <iostream>
//#include <fstream>
//#include <os/main.h>
//#include <sys/time.h>
//#include <math.h>
#include <geometry/GeometryToolkit.hpp>


class TestTcpTaskContext
    : public RTT::GenericTaskContext
{
    public:
    TestTcpTaskContext(std::string name="TestTcp");
    ~TestTcpTaskContext();
    
    protected:
    RTT::WriteDataPort<std::string> outPort; //task writes to it
    RTT::ReadDataPort<std::string> inpPort;
    RTT::ReadDataPort<ORO_Geometry::Wrench> inWrenchPort;
 
    
    RTT::Property<double> kv;
    RTT::Property<double> kw;
    RTT::Property<double> dv;
    RTT::Property<double> dw;
   
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
};

#endif
