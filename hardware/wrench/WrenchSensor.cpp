#include "WrenchSensor.hpp"
#include <rtt/Attribute.hpp>
//#include <rtt/TemplateFactories.hpp>
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>

#define CUTOFF_FREQUENCY_FILTER1 500.0
#define CUTOFF_FREQUENCY_FILTER2 125.0
#define CUTOFF_FREQUENCY_FILTER3  31.25
#define CUTOFF_FREQUENCY_FILTER4   7.813
#define CUTOFF_FREQUENCY_FILTER5   1.953
#define CUTOFF_FREQUENCY_FILTER6   0.4883

// 12000 is more or less 75% of 16384
// and 16384 is half of 32768, which is the full range (e.g. -200 to 200 N).
#define MAX_LOAD 12000

namespace Orocos
{
  using namespace RTT;
  using namespace std;
  using namespace KDL;
  
  WrenchSensor::WrenchSensor(double samplePeriod,std::string name,unsigned int DSP,string propertyfile) 
    : RTT::GenericTaskContext(name),
      outdatPort("WrenchData"),
      _filterToReadFrom(6),
      _dsp(DSP),
      _propertyfile(propertyfile),
      _offset("offset","Offset for zero-measurement",Wrench::Zero())
  {
      Toolkit::Import( KDLToolkit );
      this->ports()->addPort( &outdatPort );
      
      this->properties()->addProperty(&_offset);
      

      /**
       * Method Factory Interface.
       */
      
      _writeBuffer = new Wrench(Wrench::Zero());
      _offset.value() = Wrench::Zero();
      
      this->methods()->addMethod( method( "minMeasurement", &WrenchSensor::minMeasurement, this), "Gets the minimum measurement value."  );
      
      this->methods()->addMethod( method( "maxMeasurement", &WrenchSensor::maxMeasurement, this), "Gets the maximum measurement value."  );
      
      this->methods()->addMethod( method( "zeroMeasurement", &WrenchSensor::zeroMeasurement, this), "Gets the zero measurement value."  );
      
    this->commands()->addCommand( command( "chooseFilter", &WrenchSensor::chooseFilter,
                                           &WrenchSensor::chooseFilterDone, this),
                                  "Command to choose a different filter","p","periodValue"  );	
    
    this->commands()->addCommand( command( "setOffset", &WrenchSensor::setOffset,
                                           &WrenchSensor::setOffsetDone, this),
                                  "Command to set the zero offset","o","offset vector"  );	
    this->commands()->addCommand( command( "addOffset", &WrenchSensor::addOffset,
                                           &WrenchSensor::setOffsetDone, this),
                                  "Command to set the zero offset","o","offset vector"  );	

    this->events()->addEvent( "maximumLoadEvent", &maximumLoadEvent);
    
#if defined (OROPKG_OS_LXRT)            
    chooseFilter(samplePeriod);
    assert(JR3DSP_check_sensor_and_DSP( _dsp));
    JR3DSP_set_units(1, _dsp);
    
    JR3DSP_get_full_scale(&_full_scale, _dsp);
    
    Logger::log() << Logger::Info << "WrenchSensor -  Full scale: ("
  		  << (double) _full_scale.Fx << ", " << (double) _full_scale.Fy << ", " << (double) _full_scale.Fz
  		  << (double) _full_scale.Tx << ", " << (double) _full_scale.Ty << ", " << (double) _full_scale.Tz << ")" << Logger::endl;
#else
    _full_scale.Fx=100;
    _full_scale.Fy=100;
    _full_scale.Fz=100;
    _full_scale.Tx=100;
    _full_scale.Ty=100;
    _full_scale.Tz=100;
#endif				
    
    
    if (!readProperties(_propertyfile)) {
      Logger::log() << Logger::Error << "Failed to read the property file. Offsets are set to zero" << Logger::endl;
    }
    
    
  }
  
    Wrench WrenchSensor::maxMeasurement() const
  {
    return Wrench( Vector((double)_full_scale.Fx     , (double)_full_scale.Fy     , (double)_full_scale.Fz)
				 , Vector((double)_full_scale.Tx / 10, (double)_full_scale.Fy / 10, (double)_full_scale.Fz / 10) );
  }
  
  
  Wrench WrenchSensor::minMeasurement() const
  {
    return Wrench( Vector(-(double)_full_scale.Fx     , -(double)_full_scale.Fy     , -(double)_full_scale.Fz)
				 , Vector(-(double)_full_scale.Tx / 10, -(double)_full_scale.Fy / 10, -(double)_full_scale.Fz / 10) );
    
  }
  
  Wrench WrenchSensor::zeroMeasurement() const
  {
    return Wrench(Vector(0,0,0), Vector(0,0,0));
  }
  
  bool WrenchSensor::chooseFilter(double period)
  {
#if defined (OROPKG_OS_LXRT) 
    // Calculate the best filter to read from, based on the value 'T'
    if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER1)) _filterToReadFrom = 1;
    else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER2)) _filterToReadFrom = 2;
    else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER3)) _filterToReadFrom = 3;
    else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER4)) _filterToReadFrom = 4;
    else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER5)) _filterToReadFrom = 5;
    else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER6)) _filterToReadFrom = 6;
    else
      {
	Logger::log() << Logger::Warning << "(WrenchSensor)  Sample to low to garantee no aliasing!" << Logger::endl;
	return false;
      }
#endif 
    Logger::log() << Logger::Info << "WrenchSensor - ChooseFilter: " << _filterToReadFrom << Logger::endl;
    return true;
  }
  
  bool WrenchSensor::chooseFilterDone() const
  {
    
    return true;
  }
  
  
  bool WrenchSensor::setOffset(Wrench off)
  {
    
    _offset.value()=off;
    return true;
  }
  bool WrenchSensor::addOffset(Wrench off)
  {
    
    _offset.value()= _offset.value()+off;
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
  bool WrenchSensor::startup() {
    update(); 
    return true;
    
  }
  
  /**
   * This function is periodically called.
   */
  void WrenchSensor::update() {
    
#if defined (OROPKG_OS_LXRT)
    JR3DSP_get_data(&_write_struct, _filterToReadFrom, _dsp);
    
    // Check for overload
    if (  (_write_struct.Fx > MAX_LOAD) || (_write_struct.Fx < -MAX_LOAD)
	  || (_write_struct.Fy > MAX_LOAD) || (_write_struct.Fy < -MAX_LOAD)
	  || (_write_struct.Fz > MAX_LOAD) || (_write_struct.Fz < -MAX_LOAD)
	  || (_write_struct.Tx > MAX_LOAD) || (_write_struct.Tx < -MAX_LOAD)
	  || (_write_struct.Ty > MAX_LOAD) || (_write_struct.Ty < -MAX_LOAD)
	  || (_write_struct.Tz > MAX_LOAD) || (_write_struct.Tz < -MAX_LOAD) )
      {
	
	//EventC maxLoadEvent  = this.events()->setupEmit( "maximumLoadEvent" );
	//maxLoadEvent.emit();				       
	maximumLoadEvent.emit();
      }
#else
    _write_struct.Fx = 0;
    _write_struct.Fy = 0;
    _write_struct.Fz = 0;
    _write_struct.Tx = 0;
    _write_struct.Ty = 0;
    _write_struct.Tz = 0;
#endif
  	
    // Scale and copy to _writeBuffer (a Wrench)
    // Measurements are in a right turning coordinate system (force exerted BY the sensor),
    // so switch Fy and Ty sign to get left turning measurement.
    
    (*_writeBuffer)(0) =   (double)_write_struct.Fx * (double) _full_scale.Fx / 16384.0;
    (*_writeBuffer)(1) = - (double)_write_struct.Fy * (double) _full_scale.Fy / 16384.0;
    (*_writeBuffer)(2) =   (double)_write_struct.Fz * (double) _full_scale.Fz / 16384.0;
    // All the torques are in dNm (Nm*10), so scale:							        
    (*_writeBuffer)(3) =   (double)_write_struct.Tx * (double) _full_scale.Tx / 16384.0 / 10;
    (*_writeBuffer)(4) = - (double)_write_struct.Ty * (double) _full_scale.Ty / 16384.0 / 10;         
    (*_writeBuffer)(5) =   (double)_write_struct.Tz * (double) _full_scale.Tz / 16384.0 / 10;
    
    outdatPort.Set( *_writeBuffer - _offset.value() );
  }
  
  /**
   * This function is called when the task is stopped.
   */
  void WrenchSensor::shutdown() {
    writeProperties(_propertyfile);
  }
  
  WrenchSensor::~WrenchSensor() {
    if(_writeBuffer)
      delete _writeBuffer;
  }
}//namespace




