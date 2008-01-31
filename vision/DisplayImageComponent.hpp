/***************************************************************************

                        ImageDisplayComponent.hpp -  description
                        -------------------------

    begin                : Jan 2008                                                                                                                                  
    copyright            : (C) 2008 Francois Cauwe
    based on             : ReporterComponent.cpp made by Peter Soetens
    email                : francois@cauwe.org
                                                                                                                                                                           
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

#ifndef __IMAGEDISPLAYCOMPONENT__
#define __IMAGEDISPLAYCOMPONENT__

// RTT includes
#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>

// OpenCV includes for displaying
#include <opencv/cv.h>
#include <opencv/highgui.h>


#include "Vision.hpp"

namespace OCL
{
    /** 
     * @brief A component for periodically displaying IplImages
     * of other components. 
     *
     * WARNING: You should only use one ImageDisplayComponent, and 
     * OpenCV's mainloop (waitkey()) should not be called by an other 
     * component.
     * 
     * @par Configuration
     * The ImageDisplayComponent (and its decendants) take two configuration
     * files. The first is the 'classical' XML '.cpf' file which contains
     * the property values of the ImageDisplayComponent. The second XML file
     * is in the same format, but describes which ports and peer components 
     * need to be monitored. It has the following format:
     * @code
     <?xml version="1.0" encoding="UTF-8"?>
     <!DOCTYPE properties SYSTEM "cpf.dtd">
     <properties>

       <!-- Monitor a single Data or Buffer-Port of another Component : -->
       <simple name="ImagePort" type="string"><description></description><value>ComponentY.PortZ</value></simple>

       <!-- add as many lines as desired... -->

     </properties>
     @endcode
     * The above file is read by the 'ReportingComponent::configure()' method, the file to load
     * is listed in the 'Configuration' property.
     */

    class  ImageDisplayComponent : public RTT::TaskContext
    {
    public:
        /**
         * Set up a component for displaying.
         */
        ImageDisplayComponent(std::string name);
        
        virtual ~ImageDisplayComponent();
        
        /**
         * Implementation of TaskCore::configureHook().
         */
        virtual bool configureHook();
        
	// do nothing
        virtual bool startHook();

        /**
         * This not real-time function displays the dataports the copied data.
         */
        virtual void updateHook();
        
	// do nothing
        virtual void stopHook();
      
        /**
         * Implementation of TaskCore::cleanupHook().
         * Clears the configuration.
         */
        virtual void cleanupHook();
        
        
    private:
        
        typedef boost::tuple<std::string,
                             RTT::DataSourceBase::shared_ptr,
                             boost::shared_ptr<RTT::CommandInterface>,
                             RTT::DataSourceBase::shared_ptr,
                             std::string> DTupple;
        /** 
         * Stores the 'datasource' of all displayed items as properties.
         */
        typedef std::vector<DTupple> Reports;
        Reports root;
        
        virtual bool addDisplayPort(std::string& cname, std::string& pname);
        
        bool addDisplaySource(std::string tag, std::string type, RTT::DataSourceBase::shared_ptr orig);
        
        RTT::Property<std::string>   config;
        
        
    }; 
}
#endif  // IMAGEDISPLAYCOMPONENT
