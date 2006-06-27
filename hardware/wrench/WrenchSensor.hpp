#ifndef WRENCH_SENSOR_H
#define WRENCH_SENSOR_H

#include <execution/GenericTaskContext.hpp>
#include <corelib/Attribute.hpp>
#include <corelib/Event.hpp>
#include <execution/TemplateFactories.hpp>
#include <execution/Ports.hpp>
#include <geometry/frames.h>
#include <geometry/GeometryToolkit.hpp>

#include <pkgconf/system.h> 
#if defined (OROPKG_OS_LXRT)

#include "JR3_lxrt_user.h"

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
    RTT::WriteDataPort<ORO_Geometry::Wrench> outdatPort;
    
    /**
     * Task's Methods.
     */
    virtual ORO_Geometry::Wrench maxMeasurement() const;
    virtual ORO_Geometry::Wrench minMeasurement() const;
    virtual ORO_Geometry::Wrench zeroMeasurement() const;
    
    
    virtual bool chooseFilter(double period); 
    virtual bool chooseFilterDone() const;
    
    virtual bool setOffset(ORO_Geometry::Wrench); 
    virtual bool addOffset(ORO_Geometry::Wrench); 
    virtual bool setOffsetDone() const;
    
    RTT::Event<void(void)> maximumLoadEvent;
    
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
    
  private:
    
    unsigned int  _filterToReadFrom;
    unsigned int  _dsp;
    std::string   _propertyfile;
    
    
    ORO_Geometry::Wrench*  _writeBuffer;
    RTT::Property<ORO_Geometry::Wrench>   _offset;  
    s16Forces              _write_struct,_full_scale;
    
  };
}//namespace

#endif
