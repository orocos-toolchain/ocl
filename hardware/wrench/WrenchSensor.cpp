// Copyright  (C)  2007  Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// Version: 1.0
// Author: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// Maintainer: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// URL: http://www.orocos.org/ocl

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


#include "WrenchSensor.hpp"
#include <kdl/frames_io.hpp>
#include <rtt/Logger.hpp>
#include <ocl/ComponentLoader.hpp>

ORO_CREATE_COMPONENT( OCL::WrenchSensor );

#if defined (OROPKG_OS_LXRT)
#include <hardware/wrench/drivers/jr3_lxrt_user.h>
#endif

#define CUTOFF_FREQUENCY_FILTER1 500.0
#define CUTOFF_FREQUENCY_FILTER2 125.0
#define CUTOFF_FREQUENCY_FILTER3  31.25
#define CUTOFF_FREQUENCY_FILTER4   7.813
#define CUTOFF_FREQUENCY_FILTER5   1.953
#define CUTOFF_FREQUENCY_FILTER6   0.4883

// 12000 is more or less 75% of 16384
// and 16384 is half of 32768, which is the full range (e.g. -200 to 200 N).
#define MAX_LOAD 12000

namespace OCL
{
    using namespace RTT;
    using namespace std;
    using namespace KDL;
    
    WrenchSensor::WrenchSensor(std::string name) 
        : RTT::TaskContext(name,PreOperational),
          outdatPort("WrenchData"),
          maximumLoadEvent("maximumLoadEvent"), 
          maxMeasurement_mtd( "maxMeasurement", &WrenchSensor::maxMeasurement, this),
          minMeasurement_mtd( "minMeasurement", &WrenchSensor::minMeasurement, this),
          zeroMeasurement_mtd( "zeroMeasurement", &WrenchSensor::zeroMeasurement, this),
          chooseFilter_cmd( "chooseFilter", &WrenchSensor::chooseFilter,
                        &WrenchSensor::chooseFilterDone, this),
          setOffset_cmd( "setOffset", &WrenchSensor::setOffset,
                     &WrenchSensor::setOffsetDone, this),
          addOffset_cmd( "addOffset", &WrenchSensor::addOffset,
                     &WrenchSensor::setOffsetDone, this),
          offset("offset","Offset for zero-measurement",Wrench::Zero()),
          dsp_prop("dsp","connection socket, can be 0 or 1",0),
          filter_period_prop("filterperiod","period of filter for measurements",1.0/(2*CUTOFF_FREQUENCY_FILTER6)),
          filterToReadFrom(6)
    {
        this->ports()->addPort( &outdatPort );
        this->properties()->addProperty(&offset);
        this->properties()->addProperty(&dsp_prop);
        this->properties()->addProperty(&filter_period_prop);
      
        /**
         * Method Factory Interface.
         */
        this->methods()->addMethod( &minMeasurement_mtd, "Gets the minimum measurement value."  );
        this->methods()->addMethod( &maxMeasurement_mtd, "Gets the maximum measurement value."  );
        this->methods()->addMethod( &zeroMeasurement_mtd, "Gets the zero measurement value."  );

        this->commands()->addCommand( &chooseFilter_cmd, "Command to choose a different filter","p","periodValue"  );	
        this->commands()->addCommand( &setOffset_cmd, "Command to set the zero offset","o","offset wrench"  );	
        this->commands()->addCommand( &addOffset_cmd, "Command to add an offset","o","offset wrench"  );	
    
        this->events()->addEvent(&maximumLoadEvent, "Maximum Load","message","Information about event");
        
    }

    bool WrenchSensor::configureHook()
    {
        dsp = dsp_prop.value();
#if defined (OROPKG_OS_LXRT)            
        chooseFilter(filter_period_prop.value());
        if(!JR3DSP_check_sensor_and_DSP(dsp)){
            log(Error)<<"WrenchSensor not plugged in connected!!!!"<<endlog();
            return false;
        }
        
        JR3DSP_set_units(1, dsp);
        
        JR3DSP_get_full_scale(&full_scale, dsp);
        
        log(Info) << "WrenchSensor -  Full scale: ("
                  << (double) full_scale.Fx << ", " << (double) full_scale.Fy << ", " << (double) full_scale.Fz
                  << (double) full_scale.Tx << ", " << (double) full_scale.Ty << ", " << (double) full_scale.Tz 
                  << ")" << endlog();
#else
        full_scale.Fx=100;
        full_scale.Fy=100;
        full_scale.Fz=100;
        full_scale.Tx=100;
        full_scale.Ty=100;
        full_scale.Tz=100;
#endif	
		return true;
        
    }
    
  
    Wrench WrenchSensor::maxMeasurement() const
    {
        return Wrench( Vector((double)full_scale.Fx     , (double)full_scale.Fy     , (double)full_scale.Fz),
                       Vector((double)full_scale.Tx / 10, (double)full_scale.Fy / 10, (double)full_scale.Fz / 10) );
    }
    
    Wrench WrenchSensor::minMeasurement() const
    {
        return Wrench( Vector(-(double)full_scale.Fx     , -(double)full_scale.Fy     , -(double)full_scale.Fz),
                       Vector(-(double)full_scale.Tx / 10, -(double)full_scale.Fy / 10, -(double)full_scale.Fz / 10) );
    }
  
    Wrench WrenchSensor::zeroMeasurement() const
    {
        return Wrench(Vector(0,0,0), Vector(0,0,0));
    }
  
    bool WrenchSensor::chooseFilter(double period)
    {
#if defined (OROPKG_OS_LXRT) 
        // Calculate the best filter to read from, based on the value 'T'
        if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER1)) filterToReadFrom = 1;
        else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER2)) filterToReadFrom = 2;
        else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER3)) filterToReadFrom = 3;
        else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER4)) filterToReadFrom = 4;
        else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER5)) filterToReadFrom = 5;
        else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER6)) filterToReadFrom = 6;
        else
            {
                log(Warning) << "(WrenchSensor)  Sample to low to garantee no aliasing!" << endlog();
                return false;
            }
#endif 
        log(Info) << "WrenchSensor - ChooseFilter: " << filterToReadFrom << endlog();
        return true;
    }
    
    bool WrenchSensor::chooseFilterDone() const
    {
        return true;
    }
  
    
    bool WrenchSensor::setOffset(Wrench off)
    {
        offset.value()=off;
        return true;
    }
    
    bool WrenchSensor::addOffset(Wrench off)
    {
        offset.value()+=off;
        return true;
    }
    
    bool WrenchSensor::setOffsetDone() const
    {
        return true;
    } 
    
    /**
     * This function contains the application's startup code.
     * Return false to abort startup.
     */
    bool WrenchSensor::startHook() {
        this->update(); 
        return true;
    }
    
    /**
     * This function is periodically called.
     */
    void WrenchSensor::updateHook() {
#if defined (OROPKG_OS_LXRT)
        JR3DSP_get_data(&write_struct, filterToReadFrom, dsp);
        
        // Check for overload
        if (  (write_struct.Fx > MAX_LOAD) || (write_struct.Fx < -MAX_LOAD)
              || (write_struct.Fy > MAX_LOAD) || (write_struct.Fy < -MAX_LOAD)
              || (write_struct.Fz > MAX_LOAD) || (write_struct.Fz < -MAX_LOAD)
              || (write_struct.Tx > MAX_LOAD) || (write_struct.Tx < -MAX_LOAD)
              || (write_struct.Ty > MAX_LOAD) || (write_struct.Ty < -MAX_LOAD)
              || (write_struct.Tz > MAX_LOAD) || (write_struct.Tz < -MAX_LOAD) )
            maximumLoadEvent("Maximum load of wrench sensor exceeded");
#else
        write_struct.Fx = 0;
        write_struct.Fy = 0;
        write_struct.Fz = 0;
        write_struct.Tx = 0;
        write_struct.Ty = 0;
        write_struct.Tz = 0;
#endif
        
        // Scale and copy to _writeBuffer (a Wrench)
        // Measurements are in a right turning coordinate system (force exerted BY the sensor),
        // so switch Fy and Ty sign to get left turning measurement.
        
        writeBuffer(0) =   (double)write_struct.Fx * (double) full_scale.Fx / 16384.0;
        writeBuffer(1) = - (double)write_struct.Fy * (double) full_scale.Fy / 16384.0;
        writeBuffer(2) =   (double)write_struct.Fz * (double) full_scale.Fz / 16384.0;
        // All the torques are in dNm (Nm*10), so scale:							        
        writeBuffer(3) =   (double)write_struct.Tx * (double) full_scale.Tx / 16384.0 / 10;
        writeBuffer(4) = - (double)write_struct.Ty * (double) full_scale.Ty / 16384.0 / 10;         
        writeBuffer(5) =   (double)write_struct.Tz * (double) full_scale.Tz / 16384.0 / 10;
        
        outdatPort.Set( writeBuffer - offset.value() );
    }
    
    /**
     * This function is called when the task is stopped.
     */
    void WrenchSensor::stopHook() {
        marshalling()->writeProperties(this->getName()+".cpf");
    }
    
}//namespace




