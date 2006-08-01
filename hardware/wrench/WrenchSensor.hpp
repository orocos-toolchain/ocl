#ifndef WRENCH_SENSOR_H
#define WRENCH_SENSOR_H

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/Event.hpp>
#include <rtt/TemplateFactories.hpp>
#include <rtt/Ports.hpp>
#include <kdl/toolkit.hpp>

#include <pkgconf/system.h> 
#if defined (OROPKG_OS_LXRT)

#include "driver/JR3_lxrt_user.h"

#else

struct s16Forces
{
    short   Fx, Fy, Fz, Tx, Ty, Tz; // Signed 16 bit
};

#endif

namespace Orocos
{
  class WrenchSensor : public RTT::GenericTaskContext
  {
    /**
     * Task's Data Ports.
     */
  public:
    WrenchSensor(double samplePeriod, std::string name="WrenchSensor",unsigned int DSP=0,std::string propertyfile="cpf/WrenchSensor.cpf");
    virtual ~WrenchSensor();
    
  protected:
    RTT::WriteDataPort<KDL::Wrench> outdatPort;
    
    /**
     * Task's Methods.
     */
    virtual KDL::Wrench maxMeasurement() const;
    virtual KDL::Wrench minMeasurement() const;
    virtual KDL::Wrench zeroMeasurement() const;
    
    
    virtual bool chooseFilter(double period); 
    virtual bool chooseFilterDone() const;
    
    virtual bool setOffset(KDL::Wrench); 
    virtual bool addOffset(KDL::Wrench); 
    virtual bool setOffsetDone() const;
    
    RTT::Event<void(void)> maximumLoadEvent;
    
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
    
  private:
    
    unsigned int  _filterToReadFrom;
    unsigned int  _dsp;
    std::string   _propertyfile;
    
    
    KDL::Wrench*  _writeBuffer;
    RTT::Property<KDL::Wrench>   _offset;  
    s16Forces              _write_struct,_full_scale;
    
  };
}//namespace

#endif
