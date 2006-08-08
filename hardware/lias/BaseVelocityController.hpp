#ifndef BASE_VELOCITY_CONTROLLER_HPP
#define BASE_VELOCITY_CONTROLLER_HPP

/***************************************************************************
                        BaseVelocityController.cpp 
                       ----------------------------
    begin                : July 2006
    copyright            : (C) 2006 Erwin Aertbelien
    email                : firstname.lastname@mech.kuleuven.ac.be

 History (only major changes)( AUTHOR-Description ) :
        Adaptation of the file initially created by Dan Lihtetski, july 2006.
 
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




#include <rtt/RTT.hpp>
#include <rtt/GenericTaskContext.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/Ports.hpp>
#include "liasclient.hpp"

class BaseVelocityController 
    : public RTT::GenericTaskContext
{
    /**
     * Task's Data Ports.
     */
    public:
    BaseVelocityController(const std::string& name="BaseVelocityController",const std::string& propfile="cpf/base.cpf");
    
    protected:
    RTT::ReadDataPort<double> velocity;
    RTT::ReadDataPort<double> rotvel;

    RTT::WriteDataPort<double> x_world_pf;
    RTT::WriteDataPort<double> y_world_pf;
    RTT::WriteDataPort<double> theta_world_pf;

    /**
     * Task's Properties.
     */
    
    RTT::Property<std::string> hostname;
    RTT::Property<int> port;
    RTT::Property<bool> logging;

    LiasClientN::Client *cl;
    bool connected, initialiseC, setVelocityC, stopnowC, startLaserScannerC, stopLaserScannerC;
    /**
     * Task's Commands.
     */

    virtual bool initialise(double v1,double v2,double v3);
    virtual bool initialiseDone(double v1,double v2,double v3) const;

    virtual bool setVelocity(double v1,double v2);
    virtual bool setVelocityDone() const;

    virtual bool stopnow() ;
    virtual bool stopnowDone() const;
        
    virtual bool startLaserScanner() ;
    virtual bool startLaserScannerDone() const;
    
    virtual bool stopLaserScanner() ;
    virtual bool stopLaserScannerDone() const;
        
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
};

#endif
