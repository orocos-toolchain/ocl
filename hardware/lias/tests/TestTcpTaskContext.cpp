#include "TestTcpTaskContext.hpp"
#include <rtt/Attribute.hpp>
#include <rtt/Ports.hpp>
#include <kdl/frames.hpp>
#include <math.h>

using namespace RTT;
using namespace KDL;
using namespace std;

TestTcpTaskContext::TestTcpTaskContext(std::string name)
        :
      RTT::TaskContext(name),
      velocity("basevelocity"),
      rotvel  ("baserotvel"),
      inpPort("Position"),
	  inWrenchPort("WrenchData"),
	  kv("kv","Translation velocity multiplication constant",0.1),
	  kw("kw","Rotation velocity multipilcation constant",0.1),
	  dv("dv","Dead zone for translational velocity",0.6),
	  dw("dw","Dead zone for rotational velocity",0.6)
    {
        this->ports()->addPort( &inpPort      );
        this->ports()->addPort( &velocity     );
        this->ports()->addPort( &rotvel       );
	this->ports()->addPort( &inWrenchPort );
        this->attributes()->addProperty( &kv  );
        this->attributes()->addProperty( &kw  );
        this->attributes()->addProperty( &dv  );
        this->attributes()->addProperty( &dw  );
    }


   /**
     * This function contains the application's startup code.
     * Return false to abort startup.
     */
bool TestTcpTaskContext::startup() {
            // Write initial values.
            //outPort.data()->Set( "Initialise 0 0 0 \n" );
           return true;
        }

        /**
         * This function is periodically called.
         */
void TestTcpTaskContext::update()
        {
        // Read the inbound ports
        stringstream sstr;
        std::string str;

        Frame F_fs_wb(Rotation::Identity(), Vector(0.24,0.70, 0));
        Wrench w_wb = F_fs_wb.Inverse(inWrenchPort.Get());

        double force   =  w_wb(1);
	    double torque  = -w_wb(5);

        //code for converting from force to velocity
	    if (fabs(force)<dv) force=0;
	    if (fabs(torque)<dw) torque=0;

        double v=kv*force;
        double w=kw*torque;

        velocity.Set(v);
        rotvel.Set(w);
        inpPort.data()->Get(); //the position is in cm/s, have to transform in m/s
    }

    /**
     * This function is called when the task is stopped.
     */
void TestTcpTaskContext::shutdown() {

    }

TestTcpTaskContext::~TestTcpTaskContext()
{
}
