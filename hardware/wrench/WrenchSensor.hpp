#ifndef WRENCH_SENSOR_H
#define WRENCH_SENSOR_H

#include <rtt/TaskContext.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/Event.hpp>
#include <rtt/Method.hpp>
#include <rtt/Command.hpp>
#include <rtt/Ports.hpp>
#include <kdl/toolkit.hpp>
#include <kdl/frames.hpp>

#include <pkgconf/system.h> 
#if defined (OROPKG_OS_LXRT)

#include <hardware/wrench/driver/jr3_lxrt_common.h>

#else

struct s16Forces
{
    short   Fx, Fy, Fz, Tx, Ty, Tz; // Signed 16 bit
};

#endif

namespace Orocos
{
    /**
     * This class implements a TaskContext that communicates with a
     * JR3-WrenchSensor. It should be executed by a periodic activity.
     * Real sensor information is only supplied if the activity is
     * running in lxrt and the jr3-driver is succesfully loaded.
     */

    class WrenchSensor : public RTT::TaskContext
    {
        /**
         * Task's Data Ports.
         */
    public:
        /** 
         * The constructor of the component.
         * 
         * @param samplePeriod period of the filter which should be used
         * @param name name of the TaskContext (default: WrenchSensor)
         * @param DSP number of connector that should be used (some
         * pci-cards have two connections)
         * @param propertyfile location of propertyfile (default: cpf/WrenchSensor.cpf)
         * 
         */
        WrenchSensor(double samplePeriod, std::string name="WrenchSensor",unsigned int DSP=0,std::string propertyfile="cpf/WrenchSensor.cpf");
        virtual ~WrenchSensor();
        
    protected:
        virtual bool startup();
        virtual void update();
        virtual void shutdown();

        /// DataPort which contains Wrench information
        RTT::WriteDataPort<KDL::Wrench> outdatPort;
        
        /// Event that is fired if the measured force exceeds the
        /// allowed maximum value.
        RTT::Event<void(std::string)> maximumLoadEvent;
        
        /// Method to get the maximum measurement value
        RTT::Method<KDL::Wrench(void)> _maxMeasurement;
        /// Method to get the minimum measurement value
        RTT::Method<KDL::Wrench(void)> _minMeasurement;
        /// Method to get the zero measurement value
        RTT::Method<KDL::Wrench(void)> _zeroMeasurement;
        /// Command to choose a different filter
        RTT::Command<bool(double)> _chooseFilter;
        /// Method to set the zero measurement offset
        RTT::Command<bool(KDL::Wrench)> _setOffset;
        /// Method to add an offset to the zero measurement
        RTT::Command<bool(KDL::Wrench)> _addOffset;
        
    private:
        virtual KDL::Wrench maxMeasurement() const;
        virtual KDL::Wrench minMeasurement() const;
        virtual KDL::Wrench zeroMeasurement() const;
        
        virtual bool chooseFilter(double period); 
        virtual bool chooseFilterDone() const;
        
        virtual bool setOffset(KDL::Wrench); 
        virtual bool addOffset(KDL::Wrench); 
        virtual bool setOffsetDone() const;
        
        unsigned int  _filterToReadFrom;
        unsigned int  _dsp;
        std::string   _propertyfile;
    
        
        KDL::Wrench*  _writeBuffer;
        RTT::Property<KDL::Wrench>   _offset;  
        s16Forces              _write_struct,_full_scale;
        
    };
}//namespace

#endif
