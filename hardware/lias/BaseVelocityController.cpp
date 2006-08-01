/***************************************************************************
                        BaseVelocityController.cpp -  description
                       ------------------------------------------
    begin                : June 2006
    copyright            : (C) 2006 Erwin Aertbelien
    email                : firstname.lastname@mech.kuleuven.ac.be

 History (only major changes)( AUTHOR-Description ) :
    created from the initial version of Dan Lihtetski in july 2006.
 
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#include <rtt/Attribute.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Command.hpp>
#include <hardware/lias/BaseVelocityController.hpp>

using namespace RTT;
using namespace std;


BaseVelocityController::BaseVelocityController(const std::string& name,const std::string& propfile) 
        : RTT::GenericTaskContext(name),
          velocity("basevelocity"),
          rotvel  ("baserotvel"),
          outdatPort("Position"),
          hostname("hostname","The host name to connect to","10.33.172.172"),
          port("port","The port of the host computer", 9999 )
    {
        /**
         * Export ports to interface:
         */
        this->ports()->addPort( &velocity );
        this->ports()->addPort( &rotvel   );
        this->ports()->addPort( &outdatPort );
        this->properties()->addProperty( &hostname );
        this->properties()->addProperty( &port );
        
        if (!readProperties(propfile)) {
                     Logger::log() << Logger::Error << "Failed to read the property file." << Logger::endl;
                     assert(0);
         }

        /**
         * Command Interface
         */

        this->commands()->addCommand( command( "initialise", &BaseVelocityController::initialise,
                             &BaseVelocityController::initialiseDone, this),
                             "Command to initialises the coordinates of the robot.","x","position","y","position","t","position"  );

        this->commands()->addCommand( command( "setVelocity", &BaseVelocityController::setVelocity,
                             &BaseVelocityController::setVelocityDone, this),
                             "Command to set a velocity of X,T for the robot.","l","linear","r","rotational"  );

        this->commands()->addCommand( command( "stopnow", &BaseVelocityController::stopnow,
                             &BaseVelocityController::stopnowDone, this),
                             "Command to stop the robot."  );

        this->commands()->addCommand( command( "startLaserScanner", &BaseVelocityController::startLaserScanner,
                             &BaseVelocityController::startLaserScannerDone, this),
                             "Command to start the laser scanner."  );

        this->commands()->addCommand( command( "stopLaserScanner", &BaseVelocityController::stopLaserScanner,
                             &BaseVelocityController::stopLaserScannerDone, this),
                             "Command to stop the laser scanner."  );

        cl=0;        
        connected = false;
    }
  
    bool BaseVelocityController::initialise(double v1,double v2,double v3) 
    { 
   
     stringstream sstr;
     if(!connected) return false;

     initialiseC=false;
     sstr << "Initialise "<<v1<<" "<<v2<<" "<<v3<<" \n" ; 
        
     cl->sendCommand(sstr.str());
     if(cl->receiveData()!="") initialiseC=true; 				      
     return true; 
     
    }
    
    bool BaseVelocityController::initialiseDone(double v1,double v2,double v3) const
    { 
      return true;
    }

    bool BaseVelocityController::setVelocity(double v1,double v2) 
    { 
      stringstream sstr;
      if(!connected) return false;
  
      setVelocityC=false;
      sstr << "SetVelocity "<<v1<<" "<<v2<<" \n" ; 
        
      cl->sendCommand(sstr.str());
      if(cl->receiveData()!="") 
            return false;    
      else
            return true; 
    }
    
    bool BaseVelocityController::setVelocityDone() const
    {  				 
        return true;
    }

    bool BaseVelocityController::stopnow() 
    { 
       stringstream sstr;
       if(!connected) return false;

       stopnowC=false;
       sstr << "Stop \n" ; 
        
       cl->sendCommand(sstr.str());
       if(cl->receiveData()!="")
            return false;
       else 
            return true; 
    }
    
    bool BaseVelocityController::stopnowDone() const
    {
        return true;
    }
        
    bool BaseVelocityController::startLaserScanner() 
    { 
       stringstream sstr;
       if(!connected) return false;

       startLaserScannerC=false;   
       sstr << "StartScanner \n" ; 
        
       cl->sendCommand(sstr.str());
       if(cl->receiveData()!="") 
            return false;
       else 
            return true;
    }
    
    bool BaseVelocityController::startLaserScannerDone() const
    {
        return true;
    }
                  
    bool BaseVelocityController::stopLaserScanner() 
    { 
       stringstream sstr;
    
       if(!connected) return false;
       stopLaserScannerC=false;   
       sstr << "StartScanner \n" ; 
       
       cl->sendCommand(sstr.str());
       if(cl->receiveData()!="") 
            return false;
       else 
            return true; 
    }
    
    bool BaseVelocityController::stopLaserScannerDone() const
    {
        return true;
    }
    /**
     * This function contains the application's startup code.
     * Return false to abort startup.
     */
    bool BaseVelocityController::startup() {

       /*if ( ! indatPort.connected() || ! outdatPort.connected() ) {
            Logger::log() << Logger::Error << "Not all ports were properly connected. Aborting.!!!!"<<Logger::endl;
            if ( !indatPort.connected() )
                Logger::log() << indatPort.getName() << " not connected."<<Logger::endl;
            if ( !outdatPort.connected() )
                Logger::log() << outdatPort.getName() << " not connected."<<Logger::endl;
                   return false;
       }*/
       cl=new LiasClientN::Client;

       if (cl->connect(hostname,port)!=-1) { 
           connected=true; 
           Logger::log() << "Connection to "<<hostname<<":"<<port<<" established."<<Logger::endl;
       } else { 
           connected=false;
           Logger::log() << "Could not connect to "<<hostname<<":"<<port<<Logger::endl;
       }

   	   return connected; 
         
    }

    /**
     * This function is periodically called.
     */
    void BaseVelocityController::update() {
        stringstream sstr;
         
        if (!connected) return;
         
        double v = velocity.Get()/100;  // v is expressed in [cm]
        double w = rotvel.Get();        // w is expressed in [rad]
        
        sstr << "SetVelocityGetPosition "<< v << " " << w << " \n" ; 
        cl->sendCommand(sstr.str());
        string str = cl->receiveData(); 
        //  3 values are given back : x y theta
        //  with x,y in [cm/sec]
        //  jjjjjjj
        outdatPort.data()->Set( str );
    }

    /**
     * This function is called when the task is stopped.
     */
    void BaseVelocityController::shutdown() {
       	if (connected) {
            if (cl) {
                delete cl;
                cl=0;
            }
            Logger::log() << "Disconnected."<<Logger::endl;
            connected=false;
        }
   }
    

