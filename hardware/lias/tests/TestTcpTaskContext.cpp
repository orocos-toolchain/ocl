#include "TestTcpTaskContext.hpp"
#include <rtt/Attribute.hpp>
//#include <execution/TemplateFactories.hpp>
//#include <execution/TaskBrowser.hpp>
//#include <execution/MethodC.hpp>
//#include <execution/CommandC.hpp>
//#include <execution/EventC.hpp>
//#include <execution/ConnectionC.hpp>
#include <rtt/Ports.hpp>
#include <geometry/frames.h>
//#include <iostream>
//#include <os/main.h>
//#include <sys/time.h>
#include <math.h>

using namespace RTT;
using namespace ORO_Geometry;
using namespace std;

TestTcpTaskContext::TestTcpTaskContext(std::string name) 
        : RTT::GenericTaskContext(name),
          outPort("Velocity"),
          inpPort("Position"),
	  inWrenchPort("WrenchData"),
	  kv("kv","Translation velocity multiplication constant",0.1),
	  kw("kw","Rotation velocity multipilcation constant",0.1),
	  dv("dv","Dead zone for translational velocity",0.6),
	  dw("dw","Dead zone for rotational velocity",0.6)
    {

        this->ports()->addPort( &inpPort );

        this->ports()->addPort( &outPort );

	this->ports()->addPort( &inWrenchPort );


        this->attributes()->addProperty( &kv );
        this->attributes()->addProperty( &kw );
        this->attributes()->addProperty( &dv );
        this->attributes()->addProperty( &dw );
       
    
    }
   

   /**
     * This function contains the application's startup code.
     * Return false to abort startup.
     */
bool TestTcpTaskContext::startup() {

        if ( ! inpPort.connected() || ! outPort.connected() || ! inWrenchPort.connected() ) {
            Logger::log() << Logger::Error << "Not all ports were properly connected. Aborting."<<Logger::endl;
            if ( !inpPort.connected() )
                Logger::log() << inpPort.getName() << " not connected."<<Logger::endl;
            if ( !outPort.connected() )
                    Logger::log() << outPort.getName() << " not connected."<<Logger::endl;
            if ( !inWrenchPort.connected() )
                    Logger::log() << inWrenchPort.getName() << " not connected."<<Logger::endl;
	            return false;
            }
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
	    ORO_Geometry::Wrench w_fs;
	    double vel1=0.0,vel2=0.0,force,torque,force2;
	    

 	    w_fs = inWrenchPort.data()->Get();

        Frame F_fs_wb(Rotation::Identity(), Vector(0.24,0.70, 0));
        Wrench w_wb = F_fs_wb.Inverse(w_fs);

        force=w_wb(1);
	    torque=-w_wb(5);
	    //force2=-w_wb(0);
	      
              //code for converting from force to velocity
               
		    
	      if(fabs(force)<dv) force=0;
	      //if(fabs(force2)<dv) force2=0;
	      if(fabs(torque)<dw) torque=0;

	      

	      vel1=kv*force;
          vel2=kw*torque;
	      //vel2=kw*torque+kv*force2;
	      
 	    
  	      //------------------------------------------
	    
	    
	    
	      sstr <<vel1<<" "<<vel2 ; 
              str=sstr.str();          
              outPort.data()->Set(str);
           
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
