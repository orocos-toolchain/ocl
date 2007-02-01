// Copyright (C) 2006 Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  

#ifndef __LASERDISTANCE_HARDWARE__
#define __LASERDISTANCE_HARDWARE__

#include <rtt/RTT.hpp>

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Properties.hpp>

#define OROPKG_DEVICE_DRIVERS_COMEDI 1

#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)

#include "dev/ComediDevice.hpp"
#include "dev/ComediSubDeviceAIn.hpp"
#include <rtt/dev/AnalogInput.hpp>
#endif

#include <ocl/OCL.hpp>

namespace OCL {
    /**
     * This class implements a TaskContext which construct an
     * interface to the automated Laser initialising a link to the
     * digital IO PCI cards. It can also be used in simulation if the
     * comedi-device drivers were not available during compilation of orocos.
     */
    class LaserDistance : public RTT::TaskContext {
    public:
        /** 
         * Constructor of the class
         * 
         * @param name name of the TaskContext
         * @param nr_chan nr of channels that should be read
         * @param propertyfilename location of the
         * propertyfile. Default: cpf/LaserDistance.cpf
         * 
         */
        LaserDistance(std::string name,unsigned int nr_chan,std::string propertyfilename="cpf/LaserDistance.cpf");
        virtual~LaserDistance();
        
        virtual bool startup();
        virtual void update();
        virtual void shutdown();
    
        //double getDistance(int channel);
        
    private:
#if defined (OROPKG_DEVICE_DRIVERS_COMEDI)
        ComediDevice* _comediDev_NI6024; //NI-6024 for analog in
        ComediSubDeviceAIn* _comediSubdevAIn;
        std::vector<RTT::AnalogInput<unsigned int>*> _LaserInput;
#endif
    
        unsigned int _nr_chan;
    protected:
        /// values which should be used in simulation
        RTT::Property<std::vector<double> > _simulation_values;
        /// Conversion factor from volts to meters
        RTT::Property<std::vector<double> > _volt2m;
        /// Offset of measurement in meters
        RTT::Property<std::vector<double> > _offsets;
        /// lower limits of measurements, fires _distanceOutOfRange event
        RTT::Property<std::vector<double> > _lowerLimits;
        /// upper limits of measurements, fires _distanceOutOfRange event
        RTT::Property<std::vector<double> > _upperLimits;
        /// Dataport which contains the measurements
        RTT::WriteDataPort<std::vector<double> > _distances;
        /// Event which is fired if the distance is out of range
        RTT::Event< void(int,double)>  _distanceOutOfRange;
    private:
        std::vector<double> _measurement, _distances_local;
        std::string _propertyfile;
    }; // class
} // namespace

#endif
